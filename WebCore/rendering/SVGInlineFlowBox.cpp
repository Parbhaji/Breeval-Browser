/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 *           (C) 2006 Apple Computer Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#ifdef SVG_SUPPORT

#include "SVGInlineFlowBox.h"

#include "GraphicsContext.h"
#include "InlineTextBox.h"
#include "KCanvasRenderingStyle.h"
#include "RootInlineBox.h"
#include "SVGLengthList.h"
#include "SVGPaintServer.h"
#include "SVGResourceClipper.h"
#include "SVGResourceFilter.h"
#include "SVGResourceMasker.h"
#include "SVGTextPositioningElement.h"

using namespace std;

namespace WebCore {

void SVGInlineFlowBox::paint(RenderObject::PaintInfo& paintInfo, int tx, int ty)
{
    paintSVGInlineFlow(this, object(), paintInfo, tx, ty);
}

int SVGInlineFlowBox::placeBoxesHorizontally(int x, int& leftPosition, int& rightPosition, bool& needsWordSpacing)
{
    return placeSVGFlowHorizontally(this, x, leftPosition, rightPosition, needsWordSpacing);
}

void SVGInlineFlowBox::verticallyAlignBoxes(int& heightOfBlock)
{
    placeSVGFlowVertically(this, heightOfBlock);
}

void paintSVGInlineFlow(InlineFlowBox* flow, RenderObject* object, RenderObject::PaintInfo& paintInfo, int tx, int ty)
{
    if (paintInfo.context->paintingDisabled())
        return;

    paintInfo.context->save();
    paintInfo.context->concatCTM(object->localTransform());

    FloatRect boundingBox(tx + flow->xPos(), ty + flow->yPos(), flow->width(), flow->height());

    const SVGRenderStyle* svgStyle = object->style()->svgStyle();
    if (SVGResourceClipper* clipper = getClipperById(object->document(), svgStyle->clipPath().substring(1)))
        clipper->applyClip(paintInfo.context, boundingBox);

    if (SVGResourceMasker* masker = getMaskerById(object->document(), svgStyle->maskElement().substring(1)))
        masker->applyMask(paintInfo.context, boundingBox);

    SVGResourceFilter* filter = getFilterById(object->document(), svgStyle->filter().substring(1));
    if (filter)
        filter->prepareFilter(paintInfo.context, boundingBox);

    RenderObject::PaintInfo pi(paintInfo);

    if (!flow->isRootInlineBox())
        pi.rect = (object->localTransform()).inverse().mapRect(pi.rect);

    float opacity = object->style()->opacity();
    if (opacity < 1.0f) {
        paintInfo.context->clip(enclosingIntRect(boundingBox));
        paintInfo.context->beginTransparencyLayer(opacity);
    }

    SVGPaintServer* fillPaintServer = KSVGPainterFactory::fillPaintServer(object->style(), object);
    if (fillPaintServer) {
        if (fillPaintServer->setup(pi.context, object, ApplyToFillTargetType, true)) {
            flow->InlineFlowBox::paint(pi, tx, ty);
            fillPaintServer->teardown(pi.context, object, ApplyToFillTargetType, true);
        }
    }

    SVGPaintServer* strokePaintServer = KSVGPainterFactory::strokePaintServer(object->style(), object);
    if (strokePaintServer) {
        if (strokePaintServer->setup(pi.context, object, ApplyToStrokeTargetType, true)) {
            flow->InlineFlowBox::paint(pi, tx, ty);
            strokePaintServer->teardown(pi.context, object, ApplyToStrokeTargetType, true);
        }
    }

    if (filter)
        filter->applyFilter(paintInfo.context, boundingBox);

    if (opacity < 1.0f)
        paintInfo.context->endTransparencyLayer();

    paintInfo.context->restore();
}

static bool translateBox(InlineBox* box, int x, int y, bool topLevel = true)
{
    if (box->object()->isText()) {
        box->setXPos(box->xPos() + x);
        box->setYPos(box->yPos() + y);
    } else {
        InlineFlowBox* flow = static_cast<InlineFlowBox*>(box);
        SVGTextPositioningElement* text = static_cast<SVGTextPositioningElement*>(box->object()->element());
        
        if (topLevel || !(text->x()->getFirst().value() || text->y()->getFirst().value() ||
                          text->dx()->getFirst().value() || text->dy()->getFirst().value())) {
            box->setXPos(box->xPos() + x);
            box->setYPos(box->yPos() + y);
            for (InlineBox* curr = flow->firstChild(); curr; curr = curr->nextOnLine()) {
                if (!translateBox(curr, x, y, false))
                    return false;
            }
        }
    }
    return true;
}

static int placePositionedBoxesHorizontally(InlineFlowBox* flow, int x, int& leftPosition, int& rightPosition, int& leftAlign, int& rightAlign, bool& needsWordSpacing, int xPos, bool positioned)
{
    int mn = INT_MAX;
    int mx = INT_MIN;
    int amn = INT_MAX;
    int amx = INT_MIN;
    int startx = x;
    bool seenPositionedElement = false;
    flow->setXPos(x);
    for (InlineBox* curr = flow->firstChild(); curr; curr = curr->nextOnLine()) {
        if (curr->object()->isText()) {
            mn = min(mn, x);
            amn = min(amn, x);
            InlineTextBox* text = static_cast<InlineTextBox*>(curr);
            RenderText* rt = static_cast<RenderText*>(text->object());
            if (rt->length()) {
                if (needsWordSpacing && DeprecatedChar(rt->text()[text->start()]).isSpace())
                    x += rt->font(flow->isFirstLineStyle())->wordSpacing();
                needsWordSpacing = !DeprecatedChar(rt->text()[text->end()]).isSpace();
            }
            text->setXPos(x);
            x += text->width();
            mx = max(mx, x);
            amx = max(amx, x);
        } else {
            assert(curr->object()->isInlineFlow());
            InlineFlowBox* flow = static_cast<InlineFlowBox*>(curr);
            SVGTextPositioningElement* text = static_cast<SVGTextPositioningElement*>(flow->object()->element());
            x += (int)(text->dx()->getFirst().value());
            if (text->x()->numberOfItems() > 0)
                x = (int)(text->x()->getFirst().value() - xPos);
            if (text->x()->numberOfItems() > 0 ||
                text->y()->numberOfItems() > 0 ||
                text->dx()->numberOfItems() > 0 ||
                text->dy()->numberOfItems() > 0) {
                seenPositionedElement = true;
                needsWordSpacing = false;
                int ignoreX, ignoreY;
                x = placePositionedBoxesHorizontally(flow, x, mn, mx, ignoreX, ignoreY, needsWordSpacing, xPos, true);
            } else if (seenPositionedElement) {
                int ignoreX, ignoreY;
                x = placePositionedBoxesHorizontally(flow, x, mn, mx, ignoreX, ignoreY, needsWordSpacing, xPos, false);
            } else
                x = placePositionedBoxesHorizontally(flow, x, mn, mx, amn, amx, needsWordSpacing, xPos, false);
        }
    }

    if (mn > mx)
        mn = mx = startx;
    if (amn > amx)
        amn = amx = startx;

    int width = mx - mn;
    flow->setWidth(width);
    int awidth = amx - amn;
    int dx = 0;
    if (positioned) {
        switch (flow->object()->style()->svgStyle()->textAnchor()) {
            case TA_MIDDLE:
                translateBox(flow, dx = -awidth / 2, 0);
                break;
            case TA_END:
                translateBox(flow, dx = -awidth, 0);
                break;
            case TA_START:
            default:
                break;
        }

        if (dx) {
            x += dx;
            mn += dx;
            mx += dx;
        }
    }

    leftPosition = min(leftPosition, mn);
    rightPosition = max(rightPosition, mx);
    leftAlign = min(leftAlign, amn);
    rightAlign = max(rightAlign, amx);

    return x;
}

int placeSVGFlowHorizontally(InlineFlowBox* flow, int x, int& leftPosition, int& rightPosition, bool& needsWordSpacing)
{
    int ignoreX, ignoreY;
    x = placePositionedBoxesHorizontally(flow, x, leftPosition, rightPosition, ignoreX, ignoreY, needsWordSpacing, flow->object()->xPos(), true);
    leftPosition = min(leftPosition, x);
    rightPosition = max(rightPosition, x);
    needsWordSpacing = false;
    return x;
}

static void placeBoxesVerticallyWithAbsBaseline(InlineFlowBox* flow, int& heightOfBlock, int& minY, int& maxY, int& baseline, int yPos)
{
    for (InlineBox* curr = flow->firstChild(); curr; curr = curr->nextOnLine()) {
        if (curr->isInlineFlowBox()) {
            SVGTextPositioningElement* text = static_cast<SVGTextPositioningElement*>(curr->object()->element());
            baseline += (int)(text->dy()->getFirst().value());

            if (text->y()->numberOfItems() > 0)
                baseline = (int)(text->y()->getFirst().value() - yPos);

            placeBoxesVerticallyWithAbsBaseline(static_cast<InlineFlowBox*>(curr), heightOfBlock, minY, maxY, baseline, yPos);
        }
        const Font& font = curr->object()->font(true);
        int ascent = font.ascent();
        int position = baseline - ascent;
        int height = ascent + font.descent();
        curr->setBaseline(ascent);
        curr->setYPos(position);
        curr->setHeight(height);

        if (position < minY)
            minY = position;
        if (position + height > maxY)
            maxY = position + height;
    }

    if (flow->isRootInlineBox()) {
        flow->setYPos(minY);
        flow->setHeight(maxY - minY);
        flow->setBaseline(baseline - minY);
        heightOfBlock += maxY - minY;
    }
}

void placeSVGFlowVertically(InlineFlowBox* flow, int& heightOfBlock)
{
    int top = INT_MAX;
    int bottom = INT_MIN;
    int baseline = heightOfBlock;
    placeBoxesVerticallyWithAbsBaseline(flow, heightOfBlock, top, bottom, baseline, flow->object()->yPos());
    flow->setVerticalOverflowPositions(top, bottom);
    flow->setVerticalSelectionPositions(top, bottom);
}

} // namespace WebCore

#endif // SVG_SUPPORT
