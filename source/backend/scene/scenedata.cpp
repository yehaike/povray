//******************************************************************************
///
/// @file backend/scene/scenedata.cpp
///
/// @todo   What's in here?
///
/// @copyright
/// @parblock
///
/// Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
/// Copyright 1991-2015 Persistence of Vision Raytracer Pty. Ltd.
///
/// POV-Ray is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as
/// published by the Free Software Foundation, either version 3 of the
/// License, or (at your option) any later version.
///
/// POV-Ray is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
///
/// ----------------------------------------------------------------------------
///
/// POV-Ray is based on the popular DKB raytracer version 2.12.
/// DKBTrace was originally written by David K. Buck.
/// DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
///
/// @endparblock
///
//******************************************************************************

#include <sstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/scene/scenedata.h"

#include "core/material/pattern.h"
#include "core/shape/truetype.h"

#include "povms/povmsid.h"

#include "backend/vm/fnpovfpu.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

SceneData::SceneData() :
    fog(NULL),
    rainbow(NULL),
    skysphere(NULL),
    functionVM(NULL)
{
    atmosphereIOR = 1.0;
    atmosphereDispersion = 0.0;
    backgroundColour = ToTransColour(RGBFTColour(0.0, 0.0, 0.0, 0.0, 1.0));
    ambientLight = MathColour(1.0);

    iridWavelengths = MathColour::DefaultWavelengths();

    languageVersion = OFFICIAL_VERSION_NUMBER;
    languageVersionSet = false;
    languageVersionLate = false;
    warningLevel = 10; // all warnings
    stringEncoding = 0; // ASCII
    noiseGenerator = kNoiseGen_RangeCorrected;
    explicitNoiseGenerator = false; // scene has not set the noise generator explicitly
    numberOfWaves = 10;
    parsedMaxTraceLevel = MAX_TRACE_LEVEL_DEFAULT;
    parsedAdcBailout = 1.0 / 255.0; // adc bailout sufficient for displays
    gammaMode = kPOVList_GammaMode_None; // default setting for 3.62, which in turn is the default for the language
    workingGamma.reset();
    workingGammaToSRGB.reset();
    inputFileGammaSet = false; // TODO remove for 3.7x
    inputFileGamma = SRGBGammaCurve::Get();

    mmPerUnit = 10;
    useSubsurface = false;
    subsurfaceSamplesDiffuse = 50;
    subsurfaceSamplesSingle = 50;
    subsurfaceUseRadiosity = false;

    bspMaxDepth = 0;
    bspObjectIsectCost = bspBaseAccessCost = bspChildAccessCost = bspMissChance = 0.0f;

    Fractal_Iteration_Stack_Length = 0;
    Max_Blob_Components = 1000; // TODO FIXME - this gets set in the parser but allocated *before* that in the scene data, and if it is 0 here, a malloc may fail there because the memory requested is zero [trf]
    Max_Bounding_Cylinders = 100; // TODO FIXME - see note for Max_Blob_Components
    functionPatternCount = 0;
    boundingSlabs = NULL;

    splitUnions = false;
    removeBounds = true;

    TTFonts = NULL;

    tree = NULL;

    functionVM = new FunctionVM();
}

SceneData::~SceneData()
{
    lightSources.clear();
    lightGroupLightSources.clear();
    Destroy_Skysphere(skysphere);
    while (fog != NULL)
    {
        FOG *next = fog->Next;
        Destroy_Fog(fog);
        fog = next;
    }
    while (rainbow != NULL)
    {
        RAINBOW *next = rainbow->Next;
        Destroy_Rainbow(rainbow);
        rainbow = next;
    }
    if(boundingSlabs != NULL)
        Destroy_BBox_Tree(boundingSlabs);
    if(TTFonts != NULL)
        FreeFontInfo(TTFonts);
    // TODO: perhaps ObjectBase::~ObjectBase would be a better place
    //       to handle cleanup of individual objects ?
    Destroy_Object(objects);
    delete functionVM;

    if(tree != NULL)
        delete tree;
}

UCS2String SceneData::FindFile(POVMSContext ctx, const UCS2String& filename, unsigned int stype)
{
    vector<UCS2String> filenames;
    UCS2String foundfile;
    bool tryExactFirst;

    // if filename extension, matches one of the standard ones, try the exact name first
    // (otherwise, try it last)
    UCS2String::size_type pos = filename.find_last_of('.');
    tryExactFirst = false;
    if (pos != UCS2String::npos)
    {
        for (size_t i = 0; i < POV_FILE_EXTENSIONS_PER_TYPE; i++)
        {
            if ( ( strlen(gPOV_File_Extensions[stype].ext[i]) > 0 ) &&
                 ( filename.compare(pos,filename.length()-pos, ASCIItoUCS2String(gPOV_File_Extensions[stype].ext[i])) == 0 ) )
            {
                // match
                tryExactFirst = true;
                break;
            }
        }
    }

    // build list of files to search for
    if (tryExactFirst)
        filenames.push_back(filename);

    // add filename with extension variants to list of files to search for
    for (size_t i = 0; i < POV_FILE_EXTENSIONS_PER_TYPE; i++)
    {
        if (strlen(gPOV_File_Extensions[stype].ext[i]) > 0)
        {
            UCS2String fn(filename);
            fn += ASCIItoUCS2String(gPOV_File_Extensions[stype].ext[i]);
            filenames.push_back(fn);
        }
    }

    if (!tryExactFirst)
        filenames.push_back(filename);

#ifdef USE_SCENE_FILE_MAPPING
    // see if the file is available locally
    for(vector<UCS2String>::const_iterator i(filenames.begin()); i != filenames.end(); i++)
    {
        FilenameToFilenameMap::iterator ilocalfile(scene2LocalFiles.find(*i));

        if(ilocalfile != scene2LocalFiles.end())
            return *i;
    }

    // see if the file is available as temporary file
    for(vector<UCS2String>::const_iterator i(filenames.begin()); i != filenames.end(); i++)
    {
        FilenameToFilenameMap::iterator itempfile(scene2TempFiles.find(*i));

        if(itempfile != scene2TempFiles.end())
            return *i;
    }
#endif

    // otherwise, request to find the file
    RenderBackend::SendFindFile(ctx, sceneId, frontendAddress, filenames, foundfile);

    return foundfile;
}

IStream *SceneData::ReadFile(POVMSContext ctx, const UCS2String& origname, const UCS2String& filename, unsigned int stype)
{
    UCS2String scenefile(filename);
    UCS2String localfile;
    UCS2String fileurl;

#ifdef USE_SCENE_FILE_MAPPING
    // see if the file is available locally
    FilenameToFilenameMap::iterator ilocalfile(scene2LocalFiles.find(scenefile));

    // if available locally, open it end return
    if(ilocalfile != scene2LocalFiles.end())
        return NewIStream(ilocalfile->second.c_str(), stype);

    // now try the original name as given in the scene
    if((ilocalfile = scene2LocalFiles.find(origname)) != scene2LocalFiles.end())
        return NewIStream(ilocalfile->second.c_str(), stype);

    // see if the file is available as temporary file
    FilenameToFilenameMap::iterator itempfile(scene2TempFiles.find(scenefile));

    // if available as temporary file, open it end return
    if(itempfile != scene2TempFiles.end())
        return NewIStream(itempfile->second.c_str(), stype);

    // otherwise, request the file
    RenderBackend::SendReadFile(ctx, sceneId, frontendAddress, scenefile, localfile, fileurl);

    // if it is available locally, add it to the map and then open it
    if(localfile.length() > 0)
    {
        scene2LocalFiles[scenefile] = localfile;
        local2SceneFiles[localfile] = scenefile;

        // yes this is a hack
        scene2LocalFiles[origname] = localfile;

        return NewIStream(localfile.c_str(), stype);
    }

    // if it is available remotely ...
    if(fileurl.length() > 0)
    {
        // create a temporary file
        UCS2String tempname = POV_PLATFORM_BASE.CreateTemporaryFile();
        OStream *tempfile = NewOStream(tempname.c_str(), stype, false);

        if(tempfile == NULL)
        {
            POV_PLATFORM_BASE.DeleteTemporaryFile(tempname);
            throw POV_EXCEPTION_CODE(kCannotOpenFileErr);
        }

        // download the file from the URL
        // TODO - handle referrer
        if(POV_PLATFORM_BASE.ReadFileFromURL(tempfile, fileurl) == false)
        {
            delete tempfile;
            POV_PLATFORM_BASE.DeleteTemporaryFile(tempname);
            throw POV_EXCEPTION_CODE(kNetworkConnectionErr);
        }

        delete tempfile;

        // add the temporary file to the map
        scene2TempFiles[scenefile] = tempname;
        temp2SceneFiles[tempname] = scenefile;

        return NewIStream(tempname.c_str(), stype);
    }

    // file not found
    return NULL;
#else
    return NewIStream(filename.c_str(), stype);
#endif
}

/* TODO FIXME - this is the correct code but it has a bug. The code above is just a hack [trf]
IStream *SceneData::ReadFile(POVMSContext ctx, const UCS2String& filename, unsigned int stype)
{
    UCS2String scenefile(filename);
    UCS2String localfile;
    UCS2String fileurl;

    // see if the file is available locally
    FilenameToFilenameMap::iterator ilocalfile(scene2LocalFiles.find(scenefile));

    // if available locally, open it end return
    if(ilocalfile != scene2LocalFiles.end())
        return NewIStream(ilocalfile->second.c_str(), stype);

    // see if the file is available as temporary file
    FilenameToFilenameMap::iterator itempfile(scene2TempFiles.find(scenefile));

    // if available as temporary file, open it end return
    if(itempfile != scene2TempFiles.end())
        return NewIStream(itempfile->second.c_str(), stype);

    // otherwise, request the file
    RenderBackend::SendReadFile(ctx, sceneId, frontendAddress, scenefile, localfile, fileurl);

    // if it is available locally, add it to the map and then open it
    if(localfile.length() > 0)
    {
        scene2LocalFiles[scenefile] = localfile;
        local2SceneFiles[localfile] = scenefile;

        return NewIStream(localfile.c_str(), stype);
    }

    // if it is available remotely ...
    if(fileurl.length() > 0)
    {
        // create a temporary file
        UCS2String tempname = POV_PLATFORM_BASE.CreateTemporaryFile();
        OStream *tempfile = NewOStream(tempname.c_str(), stype, false);

        if(tempfile == NULL)
        {
            POV_PLATFORM_BASE.DeleteTemporaryFile(tempname);
            throw POV_EXCEPTION_CODE(kCannotOpenFileErr);
        }

        // download the file from the URL
        // TODO - handle referrer
        if(POV_PLATFORM_BASE.ReadFileFromURL(tempfile, fileurl) == false)
        {
            delete tempfile;
            POV_PLATFORM_BASE.DeleteTemporaryFile(tempname);
            throw POV_EXCEPTION_CODE(kNetworkConnectionErr);
        }

        delete tempfile;

        // add the temporary file to the map
        scene2TempFiles[scenefile] = tempname;
        temp2SceneFiles[tempname] = scenefile;

        return NewIStream(tempname.c_str(), stype);
    }

    // file not found
    return NULL;
}
*/
OStream *SceneData::CreateFile(POVMSContext ctx, const UCS2String& filename, unsigned int stype, bool append)
{
    UCS2String scenefile(filename);

#ifdef USE_SCENE_FILE_MAPPING
    // see if the file is available as temporary file
    FilenameToFilenameMap::iterator itempfile(scene2TempFiles.find(scenefile));

    // if available as temporary file, open it end return
    if(itempfile != scene2TempFiles.end())
        return NewOStream(itempfile->second.c_str(), stype, append);

    // otherwise, create a temporary file ...
    UCS2String tempname = POV_PLATFORM_BASE.CreateTemporaryFile();
    OStream *tempfile = NewOStream(tempname.c_str(), stype, append);

    // failed to open file
    if(tempfile == NULL)
        return NULL;

    // add the temporary file to the map
    scene2TempFiles[scenefile] = tempname;
    temp2SceneFiles[tempname] = scenefile;
#else
    // this is a workaround for the incomplete scene temp file support
    // until someone has time to finish it.

    OStream *tempfile = NewOStream(scenefile.c_str(), stype, append);
    if (tempfile == NULL)
        return NULL;
#endif

    // let the frontend know that a new file was created
    RenderBackend::SendCreatedFile(ctx, sceneId, frontendAddress, scenefile);

    return tempfile;
}

}
