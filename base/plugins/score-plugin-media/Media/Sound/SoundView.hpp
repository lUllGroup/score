#pragma once
#include <Process/LayerView.hpp>
#include <Media/AudioArray.hpp>
#include <Process/TimeValue.hpp>
#include <Media/MediaFileHandle.hpp>
#include <Process/ZoomHelper.hpp>

Q_DECLARE_METATYPE(QList<QPainterPath>)
namespace Media
{
namespace Sound
{
class LayerView;
struct WaveformComputer : public QObject
{
    Q_OBJECT
  public:
    enum action {KEEP_CUR = 0, USE_PREV, USE_NEXT, RECOMPUTE_ALL};
    Q_ENUM(action)

    LayerView& m_layer;
    WaveformComputer(LayerView& layer);
    std::atomic_bool dirty{};

    ~WaveformComputer()
    {
      m_drawThread.quit();
    }
    void stop() { m_drawThread.quit(); m_drawThread.wait(); }

  signals:
    void recompute(const MediaFileHandle*, double);
    void ready(QList<QPainterPath>, QPainterPath, double z);

  private slots:
    void on_recompute(const MediaFileHandle* data, double ratio);

  private:
    // Returns what to do depending on current density and stored density
    action compareDensity(const double density);

    // Computes a data set for the given ZoomRatio
    void computeDataSet(
        const MediaFileHandle& m_data,
        ZoomRatio ratio,
        double* densityptr,
        std::vector<std::vector<float>>& dataset);


    void drawWaveForms(const MediaFileHandle& data, ZoomRatio ratio);
    ZoomRatio m_zoom{};

    double m_prevdensity = -1;
    double m_density = -1;
    double m_nextdensity = -1;

    std::vector<std::vector<float>> m_prevdata;
    std::vector<std::vector<float>> m_curdata;
    std::vector<std::vector<float>> m_nextdata;

    QThread m_drawThread;
};

class LayerView final : public Process::LayerView
{
    Q_OBJECT

  public:
    explicit LayerView(QGraphicsItem* parent);
    ~LayerView();

    void setData(const MediaFileHandle& data);
    void recompute(ZoomRatio ratio);

  signals:
    void pressed();
    void askContextMenu(QPoint, QPointF);

  private:
    void contextMenuEvent(
        QGraphicsSceneContextMenuEvent* event) override;
    void paint_impl(QPainter*) const override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;

    void scrollValueChanged(int);

    void on_finishedDecoding();
    void on_newData();

    QPointer<const MediaFileHandle> m_data;
    QList<QPainterPath> m_paths;
    QPainterPath m_channels{};
    int m_numChan{};
    int m_sampleRate{};


    ZoomRatio m_pathZoom{};
    ZoomRatio m_zoom{};
    void printAction(long);

    WaveformComputer* m_cpt{};
};

}
}
