#include "BBitmap.h"
#include <string.h>
#include "Panic.h"

#ifndef __XTENSA__

#include <stdio.h>

#endif

BBitmap::BBitmap(TUint aWidth, TUint aHeight, TUint aDepth, TUint16 aMemoryType) {
  mROM              = EFalse;
  mWidth            = aWidth;
  mHeight           = aHeight;
  mDepth            = aDepth;
  mPitch            = mWidth;
  mColors           = 256;
  mPalette          = new TRGB[mColors];
  mPixels           = (TUint8 *) AllocMem(mWidth * mHeight, aMemoryType);
  mTransparentColor = -1;

  mDimensions.x1 = mDimensions.y1 = 0;
  mDimensions.x2 = mWidth - 1;
  mDimensions.y2 = mHeight - 1;
}

/**
 * construct a BBitmap from ROM definition
 */
struct ROMBitmap {
  TUint16 width, height, depth, bytesPerRow, paletteSize;
  TUint8  palette[1];
};

BBitmap::BBitmap(TAny *aROM, TUint16 aMemoryType) {
  ROMBitmap *bmp = (ROMBitmap *) aROM;

  mROM     = ETrue;
  mWidth   = bmp->width;
  mHeight  = bmp->height;
  mDepth   = bmp->depth;
  mPitch   = bmp->bytesPerRow;
  mColors  = bmp->paletteSize;
  mPalette = new TRGB[bmp->paletteSize];
  if (!mPalette) {
    Panic("Cannot allocate mPalette\n");
  }

  mDimensions.x1 = mDimensions.y1 = 0;
  mDimensions.x2 = mWidth - 1;
  mDimensions.y2 = mHeight - 1;

  mTransparentColor = -1;
  TUint8    *ptr = &bmp->palette[0];
  for (TInt i    = 0; i < bmp->paletteSize * 3; i += 3) {
    const TUint8 r = *ptr++;
    const TUint8 g = *ptr++;
    const TUint8 b = *ptr++;
    mPalette[i / 3].Set(r, g, b);
    if (r == 0xff && g == 0x00 && b == 0xff) {
      mTransparentColor = i / 3;
    }
  }

  mPixels = (TUint8 *) AllocMem(mWidth * mHeight, aMemoryType);
  if (!mPixels) {
    Panic("Cannot allocate mPixels\n");
  }
  TUint8    *dst = mPixels;
  for (TInt i    = 0; i < mHeight * mPitch; i++) {
    *dst++ = *ptr++;
  }
}

BBitmap::~BBitmap() {
  if (!mROM) {
    delete[] mPixels;
  }
  delete[] mPalette;
}

void BBitmap::Dump() {
  printf("mRom: %d\n:", mROM);
  printf("mWidth: %d, mHeight: %d, mDepth: %d, mPitch: %d\n", mWidth, mHeight, mDepth, mPitch);
  printf("mPixels: %p\n", mPixels);
  printf("mColors: %d, mPalette: %p\n", mColors, mPalette);
}

void BBitmap::SetPalette(TRGB aPalette[], TInt aCount) {
  TInt      cnt = MIN(mColors, aCount);
  for (TInt i   = 0; i < cnt; i++) {
    mPalette[i].Set(aPalette[i]);
  }
}

TBool BBitmap::DrawBitmap(BViewPort *aViewPort, BBitmap *aSrcBitmap, TRect aSrcRect, TInt aX, TInt aY,
                          TBool aFlipped, TBool aFlopped, TBool aLeft, TBool aRight) {
  TInt viewPortOffsetX = 0, viewPortOffsetY = 0;

  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Clamp x, y coords
  const TInt clampX = MIN(0, aX);
  const TInt clampY = MIN(0, aY);

  // Calculate clipped width and height
  const TInt clipW = aSrcRect.x1 < 0 ? aSrcRect.x2 : aSrcRect.Width();
  const TInt clipH = aSrcRect.y1 < 0 ? aSrcRect.y2 : aSrcRect.Height();

  // Calculate drawable width and height
  const TInt w = (clipW + clampX) - MAX(0, (clipW + aX) - clipRect.Width() + viewPortOffsetX);
  const TInt h = (clipH + clampY) - MAX(0, (clipH + aY) - clipRect.Height() + viewPortOffsetY);

  // Return if the sprite to be drawn can not be seen
  if (h <= 0) {
    return EFalse;
  }

  // Init source x,y coordinates
  const TInt sx = (aSrcRect.x1 < 0 ? aSrcRect.x1 : aSrcRect.x1 * -1) + clampX;
  const TInt sy = (aSrcRect.y1 < 0 ? aSrcRect.y1 : aSrcRect.y1 * -1) + clampY;

  // Init destination x,y coordinates
  const TInt dx = (aX < 0 ? 0 : aX) + viewPortOffsetX;
  const TInt dy = (aY < 0 ? 0 : aY) + viewPortOffsetY;

  // Calculate sprite delta width and height
  const TInt deltaImageWidth = aX < 0 ? -aSrcRect.Width() + w : aSrcRect.Width() - w;
  const TInt deltaImageHeight = aY < 0 ? -aSrcRect.Height() + h : aSrcRect.Height() - h;

  // Calculate visible width and height to iterate over
  const TInt i = -sy + h;
  const TInt j = -sx + w;

  if (aFlipped) {
    if (aFlopped) {
      // flipped and flopped
      for (TInt yy = -sy, dyy = dy, fsy = h + yy + deltaImageHeight; yy < i; yy++, dyy++, fsy--) {
        for (TInt xx = -sx, dxx = dx, fsx = w + xx + deltaImageWidth; xx < j; xx++, dxx++, fsx--) {
          // Read pixel value from bitmap
          TUint8 pix = aSrcBitmap->ReadPixel(fsx, fsy);

          // Write pixel values
          WritePixel(dxx, dyy, pix);
        }
      }
    } else {
      // flipped
      for (TInt yy = -sy, dyy = dy; yy < i; yy++, dyy++) {
        for (TInt xx = -sx, dxx = dx, fsx = w + xx + deltaImageWidth; xx < j; xx++, dxx++, fsx--) {
          // Read pixel value from bitmap
          TUint8 pix = aSrcBitmap->ReadPixel(fsx, yy);

          // Write pixel values
          WritePixel(dxx, dyy, pix);
        }
      }
    }
  } else if (aFlopped) {
    // flopped
    for (TInt yy = -sy, dyy = dy, fsy = h + yy + deltaImageHeight; yy < i; yy++, dyy++, fsy--) {
      for (TInt xx = -sx, dxx = dx; xx < j; xx++, dxx++) {
        // Read pixel value from bitmap
        TUint8 pix = aSrcBitmap->ReadPixel(xx, fsy);

        // Write pixel values
        WritePixel(dxx, dyy, pix);
      }
    }
  } else {
    // just draw
    for (TInt yy = -sy, dyy = dy; yy < i; yy++, dyy++) {
      for (TInt xx = -sx, dxx = dx; xx < j; xx++, dxx++) {
        // Read pixel value from bitmap
        TUint8 pix = aSrcBitmap->ReadPixel(xx, yy);

        // Write pixel values
        WritePixel(dxx, dyy, pix);
      }
    }
  }

  return ETrue;
}

TBool
BBitmap::DrawSprite(BViewPort *aViewPort, TInt16 aBitmapNumber, TInt aImageNumber, TInt aX, TInt aY, TUint32 aFlags) {
  BBitmap *b      = resourceManager.GetBitmap(aBitmapNumber);
  TInt    bw      = resourceManager.BitmapWidth(aBitmapNumber),
          bh      = resourceManager.BitmapHeight(aBitmapNumber),
          pitch   = b->mWidth / bw,
          viewPortOffsetX = 0,
          viewPortOffsetY = 0;

  TRect imageRect;
  imageRect.x1 = (aImageNumber % pitch) * bw;
  imageRect.x2 = imageRect.x1 + bw - 1;
  imageRect.y1 = (aImageNumber / pitch) * bh;
  imageRect.y2 = imageRect.y1 + bh - 1;

  // Sprite has no transparency
  TInt t = b->mTransparentColor;
  if (t == -1) {
    return DrawBitmap(aViewPort, b, imageRect, aX, aY, TBool(aFlags & SFLAG_FLIP), TBool(aFlags & SFLAG_FLOP),
                      TBool(aFlags & SFLAG_LEFT), TBool(aFlags & SFLAG_RIGHT));
  }

  // Create the viewport
  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Clamp x, y coords
  const TInt clampX = MIN(0, aX);
  const TInt clampY = MIN(0, aY);

  // Calculate clipped width and height
  const TInt clipW = imageRect.x1 < 0 ? imageRect.x2 : imageRect.Width();
  const TInt clipH = imageRect.y1 < 0 ? imageRect.y2 : imageRect.Height();

  // Calculate drawable width and height
  const TInt w = (clipW + clampX) - MAX(0, (clipW + aX) - clipRect.Width() + viewPortOffsetX);
  const TInt h = (clipH + clampY) - MAX(0, (clipH + aY) - clipRect.Height() + viewPortOffsetY);

  // Return if the sprite to be drawn can not be seen
  if (h <= 0) {
    return EFalse;
  }

  // Init source x,y coordinates
  const TInt sx = (imageRect.x1 < 0 ? imageRect.x1 : imageRect.x1 * -1) + clampX;
  const TInt sy = (imageRect.y1 < 0 ? imageRect.y1 : imageRect.y1 * -1) + clampY;

  // Init destination x,y coordinates
  const TInt dx = (aX < 0 ? 0 : aX) + viewPortOffsetX;
  const TInt dy = (aY < 0 ? 0 : aY) + viewPortOffsetY;

  // Calculate sprite delta width and height
  const TInt deltaImageWidth = aX < 0 ? -imageRect.Width() + w : imageRect.Width() - w;
  const TInt deltaImageHeight = aY < 0 ? -imageRect.Height() + h : imageRect.Height() - h;

  // Calculate visible width and height to iterate over
  const TInt i = -sy + h;
  const TInt j = -sx + w;

  if (aFlags & SFLAG_FLIP) {
    if (aFlags & SFLAG_FLOP) {
      // flipped and flopped
      for (TInt yy = -sy, dyy = dy, fsy = h + yy + deltaImageHeight; yy < i; yy++, dyy++, fsy--) {
        for (TInt xx = -sx, dxx = dx, fsx = w + xx + deltaImageWidth; xx < j; xx++, dxx++, fsx--) {
          // Read pixel value from bitmap
          TUint8 pix = b->ReadPixel(fsx, fsy);

          // Write non-transparent pixel values
          if (pix != t) {
            WritePixel(dxx, dyy, pix);
          }
        }
      }
    } else {
      // flipped
      for (TInt yy = -sy, dyy = dy; yy < i; yy++, dyy++) {
        for (TInt xx = -sx, dxx = dx, fsx = w + xx + deltaImageWidth; xx < j; xx++, dxx++, fsx--) {
          // Read pixel value from bitmap
          TUint8 pix = b->ReadPixel(fsx, yy);

          // Write non-transparent pixel values
          if (pix != t) {
            WritePixel(dxx, dyy, pix);
          }
        }
      }
    }
  } else if (aFlags & SFLAG_FLOP) {
    // flopped
    for (TInt yy = -sy, dyy = dy, fsy = h + yy + deltaImageHeight; yy < i; yy++, dyy++, fsy--) {
      for (TInt xx = -sx, dxx = dx; xx < j; xx++, dxx++) {
        // Read pixel value from bitmap
        TUint8 pix = b->ReadPixel(xx, fsy);

        // Write non-transparent pixel values
        if (pix != t) {
          WritePixel(dxx, dyy, pix);
        }
      }
    }
  } else {
    // just draw
    for (TInt yy = -sy, dyy = dy; yy < i; yy++, dyy++) {
      for (TInt xx = -sx, dxx = dx; xx < j; xx++, dxx++) {
        // Read pixel value from bitmap
        TUint8 pix = b->ReadPixel(xx, yy);

        // Write non-transparent pixel values
        if (pix != t) {
          WritePixel(dxx, dyy, pix);
        }
      }
    }
  }

  return ETrue;
}

void BBitmap::Clear(TUint8 aColor) {
  memset(mPixels, aColor, mPitch * mHeight);
}

void BBitmap::DrawFastHLine(BViewPort *aViewPort, TInt aX, TInt aY, TUint aW, TUint8 aColor) {
  // Initial viewport offset
  TInt viewPortOffsetX = 0;
  TInt viewPortOffsetY = 0;

  // Create the viewport
  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Store viewport width/height
  const TInt clipRectWidth = clipRect.Width();
  const TInt clipRectHeight = clipRect.Height();

  // Adjust for viewport offset
  aX += viewPortOffsetX;
  aY += viewPortOffsetY;

  // Do y bounds checks
  if (aY < viewPortOffsetY || aY >= clipRectHeight) {
    return;
  }

  // last x point + 1
  TInt16 xEnd = aX + aW;

  // Check if the entire line is not on the display
  if (xEnd <= viewPortOffsetX || aX >= clipRectWidth) {
    return;
  }

  // Don't start before the left edge
  if (aX < viewPortOffsetX) {
    aX = viewPortOffsetX;
  }

  // Don't end past the right edge
  if (xEnd > clipRectWidth) {
    xEnd = clipRectWidth;
  }

  // calculate actual width (even if unchanged)
  aW = xEnd - aX;

  while (aW > 3) {
    WritePixel(aX++, aY, aColor);
    WritePixel(aX++, aY, aColor);
    WritePixel(aX++, aY, aColor);
    WritePixel(aX++, aY, aColor);
    aW -= 4;
  }

  while (aW > 0) {
    WritePixel(aX++, aY, aColor);
    aW--;
  }
}

void BBitmap::DrawFastVLine(BViewPort *aViewPort, TInt aX, TInt aY, TUint aH, TUint8 aColor) {
  // Initial viewport offset
  TInt viewPortOffsetX = 0;
  TInt viewPortOffsetY = 0;

  // Create the viewport
  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Store viewport width/height
  const TInt clipRectWidth = clipRect.Width();
  const TInt clipRectHeight = clipRect.Height();

  // Adjust for viewport offset
  aX += viewPortOffsetX;
  aY += viewPortOffsetY;

  // Do x bounds checks
  if (aX < viewPortOffsetX || aX >= clipRectWidth) {
    return;
  }

  // last y point + 1
  TInt16 yEnd = aY + aH;

  // Check if the entire line is not on the display
  if (yEnd <= viewPortOffsetY || aY >= clipRectHeight) {
    return;
  }

  // Don't start before the top edge
  if (aY < viewPortOffsetY) {
    aY = viewPortOffsetY;
  }

  // Don't end past the bottom edge
  if (yEnd > clipRectHeight) {
    yEnd = clipRectWidth;
  }

  // calculate actual height (even if unchanged)
  aH = yEnd - aY;

  while (aH > 3) {
    WritePixel(aX, aY++, aColor);
    WritePixel(aX, aY++, aColor);
    WritePixel(aX, aY++, aColor);
    WritePixel(aX, aY++, aColor);
    aH -= 4;
  }

  while (aH > 0) {
    WritePixel(aX, aY++, aColor);
    aH--;
  }
}

void BBitmap::DrawLine(BViewPort *aViewPort, TInt aX1, TInt aY1, TInt aX2, TInt aY2, TUint8 aColor) {
  TInt viewPortOffsetX = 0;
  TInt viewPortOffsetY = 0;

  // Create the viewport
  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Store viewport width/height
  const TInt clipRectWidth = clipRect.Width();
  const TInt clipRectHeight = clipRect.Height();

  // Adjust for viewport offset
  aX1 += viewPortOffsetX;
  aX2 += viewPortOffsetX;
  aY1 += viewPortOffsetY;
  aY2 += viewPortOffsetY;

  // Draw simple lines if possible
  if (aY1 == aY2) {
    if (aX2 > aX1) {
      DrawFastHLine(aViewPort, aX1, aY1, aX2 - aX1 + 1, aColor);
    } else if (aX2 < aX1) {
      DrawFastHLine(aViewPort, aX2, aY1, aX1 - aX2 + 1, aColor);
    } else {
      WritePixel(aX1, aY1, aColor);
    }
    return;
  } else if (aX1 == aX2) {
    if (aY2 > aY1) {
      DrawFastVLine(aViewPort, aX1, aY1, aY2 - aY1 + 1, aColor);
    } else {
      DrawFastVLine(aViewPort, aX1, aY2, aY1 - aY2 + 1, aColor);
    }
    return;
  }

  // Bresenham's algorithm
  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  const TBool steep = ABS(aY2 - aY1) > ABS(aX2 - aX1);

  if (steep) {
    TInt16 temp = aX1;
    aX1 = aY1;
    aY1 = temp;

    temp = aX2;
    aX2 = aY2;
    aY2 = temp;
  }

  if (aX1 > aX2) {
    TInt16 temp = aX1;
    aX1 = aX2;
    aX2 = temp;

    temp = aY1;
    aY1 = aY2;
    aY2 = temp;
  }

  const TInt16 dx = aX2 - aX1;
  const TInt16 dy = ABS(aY2 - aY1);
  const TInt8 ystep = aY1 < aY2 ? 1 : -1;

  TInt16 err = dx / 2;

  for (; aX1 <= aX2; aX1++) {
    if (steep) {
      // aY1 is X coord and aX1 is Y coord in this case
      if ((aY1 >= viewPortOffsetX) && (aY1 <= clipRectWidth) && (aX1 >= viewPortOffsetY) && (aX1 < clipRectHeight)) {
        WritePixel(aY1, aX1, aColor);
      }
    } else {
      // aX1 is X coord and aY1 is Y coord in this case
      if ((aX1 >= viewPortOffsetX) && (aX1 <= clipRectWidth) && (aY1 >= viewPortOffsetY) && (aY1 < clipRectHeight)) {
        WritePixel(aX1, aY1, aColor);
      }
    }

    err -= dy;

    if (err < 0) {
      aY1 += ystep;
      err += dx;
    }
  }
}

void BBitmap::DrawRect(BViewPort *aViewPort, TInt aX1, TInt aY1, TInt aX2, TInt aY2, TUint8 aColor) {
  DrawFastHLine(aViewPort, aX1, aY1, aX2, aColor);
  DrawFastHLine(aViewPort, aX1, aY1 + aY2 - 1, aX2, aColor);
  DrawFastVLine(aViewPort, aX1, aY1, aY2, aColor);
  DrawFastVLine(aViewPort, aX1 + aX2 - 1, aY1, aY2, aColor);
}

void BBitmap::FillRect(BViewPort *aViewPort, TInt aX1, TInt aY1, TInt aX2, TInt aY2, TUint8 aColor) {
  const TInt w = aX1 + aX2;
  for (; aX1 < w; aX1++) {
    DrawFastVLine(aViewPort, aX1, aY1, aY2, aColor);
  }
}

void BBitmap::DrawCircle(BViewPort *aViewPort, TInt aX, TInt aY, TUint r, TUint8 aColor) {
  TInt viewPortOffsetX = 0;
  TInt viewPortOffsetY = 0;

  // Create the viewport
  TRect clipRect;
  if (aViewPort) {
    aViewPort->GetRect(clipRect);
    viewPortOffsetX = aViewPort->mOffsetX;
    viewPortOffsetY = aViewPort->mOffsetY;
  } else {
    clipRect.Set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  aX += viewPortOffsetX;
  aY += viewPortOffsetY;
  TInt f = 1 - r;
  TInt ddF_x = 1;
  TInt ddF_y = -2 * r;
  TInt x = 0;
  TInt y = r;
  TInt xx1, xx2, xx3, xx4, yy1, yy2, yy3, yy4;

  SafeWritePixel(aX, aY + r, aColor);
  SafeWritePixel(aX, aY - r, aColor);
  SafeWritePixel(aX + r, aY, aColor);
  SafeWritePixel(aX - r, aY, aColor);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    xx1 = aX + x;
    xx2 = aX - x;
    xx3 = aX + y;
    xx4 = aX - y;
    yy1 = aY + y;
    yy2 = aY - y;
    yy3 = aY + x;
    yy4 = aY - x;

    SafeWritePixel(xx1, yy1, aColor);
    SafeWritePixel(xx2, yy1, aColor);
    SafeWritePixel(xx1, yy2, aColor);
    SafeWritePixel(xx2, yy2, aColor);
    SafeWritePixel(xx3, yy3, aColor);
    SafeWritePixel(xx4, yy3, aColor);
    SafeWritePixel(xx3, yy4, aColor);
    SafeWritePixel(xx4, yy4, aColor);
  }
}

void BBitmap::FillCircle(BViewPort *aViewPort, TInt aX, TInt aY, TUint r, TUint8 aColor) {
  DrawFastVLine(aViewPort, aX, aY - r, 2 * r + 1, aColor);
  FillCircleHelper(aViewPort, aX, aY, r, 3, 0, aColor);
}

void BBitmap::FillCircleHelper(BViewPort *aViewPort, TInt aX, TInt aY, TUint r, TUint sides, TUint delta, TUint8 aColor) {
  TInt f = 1 - r;
  TInt ddF_x = 1;
  TInt ddF_y = -2 * r;
  TInt x = 0;
  TInt y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    // right side
    if (sides & 0x1) {
      DrawFastVLine(aViewPort, aX + x, aY - y, 2 * y + 1 + delta, aColor);
      DrawFastVLine(aViewPort, aX + y, aY - x, 2 * x + 1 + delta, aColor);
    }

    if (sides & 0x2) { // left side
      DrawFastVLine(aViewPort, aX - x, aY - y, 2 * y + 1 + delta, aColor);
      DrawFastVLine(aViewPort, aX - y, aY - x, 2 * x + 1 + delta, aColor);
    }
  }
}
