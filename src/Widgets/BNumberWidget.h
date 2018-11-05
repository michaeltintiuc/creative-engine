#ifndef GENUS_BNUMBERWIDGET_H
#define GENUS_BNUMBERWIDGET_H

#include <BFont.h>
#include <Widgets/BWidget.h>
#include <Widgets/BSliderWidget.h>

/**
 * Widget implementing a slider UI.
 * Presents a range slider.
 * Allows the user to pick a value between start and end in increments of step.
 */

class BNumberWidget : public BWidget {
public:
    BNumberWidget(char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground = -1);

    BNumberWidget(const char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground = -1);

    ~BNumberWidget();

public:
    TInt Render(TInt aX, TInt aY);

    void Run();

public:
    virtual void Select(TFloat aValue) = 0;

protected:
    const TRange *mRange;
    TFloat mSelectedValue;
};


#endif //GENUS_BNUMBERWIDGET_H
