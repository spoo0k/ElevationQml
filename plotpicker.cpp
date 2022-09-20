//encoding koi8-r
#include "plotpicker.h"

#include <QMouseEvent>
#include <QDebug>


    PlotPicker::PlotPicker(QWidget *canvas)
        : QwtPlotPicker(canvas)
        , range( -11000, 11000 )
    {
    }

    PlotPicker::~PlotPicker()
    {
    }

    void PlotPicker::setRange(const QPair<double,double> & range)
    {
        this->range = range;
    }

    QPair<double, double> PlotPicker::getRange() const
    {
        return range;
    }

    void PlotPicker::begin()
    {
        emit beginPick();
        QwtPlotPicker::begin();
    }

    void PlotPicker::move( const QPoint & p )
    {
//        if (range.firstidisNull())
//        {
//            QPointF bounded = invTransform(p);

//            bounded.setY( IMath::bounded( bounded.y(), range.min(), range.max() ) );

//            QwtPlotPicker::move(transform(bounded));
//        }
//        else
        {
            QwtPlotPicker::move(p);
        }
    }

    bool PlotPicker::end(bool ok)
    {
        QwtPlotPicker::end(ok);
        emit endPick();
        return true;
    }


