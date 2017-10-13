#include "PortItem.hpp"
#include <Dataflow/UI/CableItem.hpp>
#include <Process/Dataflow/DataflowObjects.hpp>
#include <QDrag>
#include <QGraphicsSceneMoveEvent>
#include <QMimeData>
#include <QPainter>
#include <QCursor>
#include <QApplication>

#include <Device/Node/NodeListMimeSerialization.hpp>
#include <State/MessageListSerialization.hpp>
#include <Dataflow/Commands/EditPort.hpp>
#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <score/document/DocumentInterface.hpp>
#include <score/document/DocumentContext.hpp>
namespace Dataflow
{
PortItem::port_map PortItem::g_ports;
PortItem::PortItem(Process::Port& p, QGraphicsItem* parent)
  : QGraphicsItem{parent}
  , m_port{p}
{
  this->setCursor(QCursor());
  this->setAcceptDrops(true);
  this->setAcceptHoverEvents(true);
  this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
  this->setToolTip(p.customData());

  g_ports.insert({&p, this});

  for(auto c : CableItem::g_cables)
  {
    if(c.first->source() == &p)
    {
      c.second->setSource(this);
      cables.push_back(c.second);
    }
    else if(c.first->sink() == &p)
    {
      c.second->setTarget(this);
      cables.push_back(c.second);
    }
  }
}

PortItem::~PortItem()
{
  for(auto cable : cables)
  {
    if(cable->source() == this)
      cable->setSource(nullptr);
    if(cable->target() == this)
      cable->setTarget(nullptr);
  }
  auto it = g_ports.find(&m_port);
  if(it != g_ports.end())
    g_ports.erase(it);
}

QRectF PortItem::boundingRect() const
{
  return {-m_diam/2., -m_diam/2., m_diam, m_diam};
}

void PortItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  painter->setRenderHint(QPainter::Antialiasing, true);
  QColor c;
  switch(m_port.type)
  {
    case Process::PortType::Audio:
      c = QColor("#FFAAAA");
      break;
    case Process::PortType::Message:
      c = QColor("#AAFFAA");
      break;
    case Process::PortType::Midi:
      c = QColor("#AAAAFF");
      break;
  }

  QPen p = c;
  p.setWidth(2);
  QBrush b = c.darker();

  painter->setPen(p);
  painter->setBrush(b);
  painter->drawEllipse(boundingRect());
  painter->setRenderHint(QPainter::Antialiasing, false);
}

static PortItem* clickedPort{};
void PortItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  clickedPort = this;
  event->accept();
}

void PortItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  event->accept();
  if(QLineF(pos(), event->pos()).length() > QApplication::startDragDistance())
  {
    QDrag d{this};
    QMimeData* m = new QMimeData;
    m->setText("cable");
    d.setMimeData(m);
    d.exec();
  }
}

void PortItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  if(this->contains(event->pos()))
    emit showPanel();
  event->accept();
}

void PortItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  prepareGeometryChange();
  m_diam = 8.;
  update();
  event->accept();
}

void PortItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  event->accept();
}

void PortItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  prepareGeometryChange();
  m_diam = 6.;
  update();
  event->accept();
}

void PortItem::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
  prepareGeometryChange();
  m_diam = 8.;
  update();
  event->accept();
}

void PortItem::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
  event->accept();
}

void PortItem::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
  prepareGeometryChange();
  m_diam = 6.;
  update();
  event->accept();
}

void PortItem::dropEvent(QGraphicsSceneDragDropEvent* event)
{
  prepareGeometryChange();
  m_diam = 6.;
  update();
  auto& mime = *event->mimeData();

  if(clickedPort && this != clickedPort)
  {
    if(this->m_port.outlet != clickedPort->m_port.outlet)
    {
      emit createCable(clickedPort, this);
    }
  }

  auto& ctx = score::IDocument::documentContext(m_port);
  CommandDispatcher<> disp{ctx.commandStack};
  if (mime.formats().contains(score::mime::addressettings()))
  {
    Mime<Device::FullAddressSettings>::Deserializer des{mime};
    Device::FullAddressSettings as = des.deserialize();

    if (as.address.path.isEmpty())
      return;

    disp.submitCommand(new ChangePortAddress{m_port, State::AddressAccessor{as.address}});
  }
  else if (mime.formats().contains(score::mime::messagelist()))
  {
    Mime<State::MessageList>::Deserializer des{mime};
    State::MessageList ml = des.deserialize();
    if (ml.empty())
      return;
    auto& newAddr = ml[0].address;

    if (newAddr == m_port.address())
      return;

    if (newAddr.address.path.isEmpty())
      return;

    disp.submitCommand(new ChangePortAddress{m_port, std::move(newAddr)});
  }
  clickedPort = nullptr;
  event->accept();
}

QVariant PortItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  switch(change)
  {
    case QGraphicsItem::ItemScenePositionHasChanged:
      for(auto cbl : cables)
      {
        cbl->resize();
      }
      break;
    case QGraphicsItem::ItemVisibleHasChanged:
    case QGraphicsItem::ItemSceneHasChanged:
      for(auto cbl : cables)
      {
        cbl->check();
      }
      break;
    default:
      break;
  }

  return QGraphicsItem::itemChange(change, value);
}

}