#pragma once
#include <QWidget>

namespace orbitwatch {
// Presentation only: this component never transforms scientific scenario data.
class EarthView : public QWidget {
public:
    explicit EarthView(QWidget* parent = nullptr);
};
}
