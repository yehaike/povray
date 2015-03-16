//******************************************************************************
///
/// @file core/shape/truetype.h
///
/// This module contains all defines, typedefs, and prototypes for
/// @ref truetype.cpp.
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

#ifndef TRUETYPE_H
#define TRUETYPE_H

#include "backend/scene/objects.h"

namespace pov
{

class CSG;
class Parser;
class SceneData;

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define TTF_OBJECT (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct GlyphStruct *GlyphPtr;

struct FontFileInfo;

class TrueType : public ObjectBase
{
    public:
        GlyphPtr glyph;      /* (GlyphPtr) Pointer to the glyph */
        DBL depth;        /* Amount of extrusion */

        TrueType();
        virtual ~TrueType();

        virtual ObjectPtr Copy();

        virtual bool All_Intersections(const Ray&, IStack&, TraceThreadData *);
        virtual bool Inside(const Vector3d&, TraceThreadData *) const;
        virtual void Normal(Vector3d&, Intersection *, TraceThreadData *) const;
        virtual void Translate(const Vector3d&, const TRANSFORM *);
        virtual void Rotate(const Vector3d&, const TRANSFORM *);
        virtual void Scale(const Vector3d&, const TRANSFORM *);
        virtual void Transform(const TRANSFORM *);
        virtual void Compute_BBox();

        static void ProcessNewTTF(CSG *Object, const char *filename, const int font_id, const UCS2 *text_string, DBL depth, const Vector3d& offset, Parser *parser, shared_ptr<SceneData>& sceneData);
    protected:
        bool Inside_Glyph(double x, double y, const GlyphStruct* glyph) const;
        int solve_quad(double *x, double *y, double mindist, DBL maxdist) const;
        void GetZeroOneHits(const GlyphStruct* glyph, const Vector3d& P, const Vector3d& D, DBL glyph_depth, double *t0, double *t1) const;
        bool GlyphIntersect(const Vector3d& P, const Vector3d& D, const GlyphStruct* glyph, DBL glyph_depth, const BasicRay &ray, IStack& Depth_Stack, TraceThreadData *Thread);
};

void FreeFontInfo(FontFileInfo *ffi);

}

#endif
