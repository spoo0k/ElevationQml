#ifndef PLOTPICKER_H
#define PLOTPICKER_H

#include <QPair>

#include <qwt_plot_picker.h>

    class PlotPicker
            : public QwtPlotPicker
    {
        Q_OBJECT
    public:
        PlotPicker(QWidget *canvas);
        ~PlotPicker();

        void setRange(const QPair<double,double> & range);
        QPair<double,double> getRange() const;

        QwtText trackerTextF(const QPointF &p) const
        {
            return QwtText(QString("D: %1, H: %2").arg(qRound(p.x())).arg(qRound(p.y())));
        }

    protected:
        void begin();
        virtual void move( const QPoint & );
        bool end(bool ok);

    signals:
        void beginPick();
        void endPick();

    private:
        QPair<double,double> range;
    };

#endif // PLOTPICKER_H
