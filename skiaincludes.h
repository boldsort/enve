#ifndef SKIAINCLUDES_H
#define SKIAINCLUDES_H

#undef foreach

#include "skiadefines.h"

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SDL.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkGradientShader.h"
#include "SkPixelRef.h"
#include "SkDashPathEffect.h"
#include "SkTypeface.h"

#include "gl/GrGLUtil.h"

#include "gl/GrGLTypes.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLInterface.h"

#include <GL/gl.h>

#include "AddInclude/SkStroke.h"

#define foreach Q_FOREACH

#endif // SKIAINCLUDES_H