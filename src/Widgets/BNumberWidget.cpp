#include "BNumberWidget.h"
#include "Display.h"
#include "Controls.h"

BNumberWidget::BNumberWidget(char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground)
    : BWidget(aTitle), mRange(aRange) {
  mSelectedValue = mRange->start;
}

BNumberWidget::BNumberWidget(const char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground)
    : BWidget(), mRange(aRange) {
  mSelectedValue = mRange->start;
  mTitle = (char *) aTitle;
  mHeight = 40;
}

BNumberWidget::~BNumberWidget() {}

TInt BNumberWidget::Render(TInt aX, TInt aY) {
  const BFont *font    = gWidgetTheme.GetFont(WIDGET_TEXT_FONT);
  const TInt  fg       = gWidgetTheme.GetInt(WIDGET_TEXT_FG),
              bg       = gWidgetTheme.GetInt(WIDGET_TEXT_BG);

  char buffer[64];
  snprintf(buffer, sizeof buffer, "%.2f", mSelectedValue);
  gDisplay.renderBitmap->DrawString(ENull, (const char*)&buffer, font, aX * 2, aY, bg, fg);

  return font->mHeight;
}

void BNumberWidget::Run() {
  // move cursor, select, etc.
  if (IsActive()) {
    if (gControls.WasPressed(JOYLEFT)) {
      mSelectedValue = MAX(mRange->start, mSelectedValue - mRange->step);
    } else if (gControls.WasPressed(JOYRIGHT)) {
      mSelectedValue = MIN(mRange->end, mSelectedValue + mRange->step);
    } else if (gControls.WasPressed(BUTTON_SELECT)) {
      Select(mSelectedValue);
    }
  }
}
