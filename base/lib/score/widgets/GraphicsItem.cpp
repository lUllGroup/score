// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <QGraphicsItem>
#include <QGraphicsScene>

#include "GraphicsItem.hpp"

void deleteGraphicsObject(QGraphicsObject* item)
{
  if (item)
  {
    auto sc = item->scene();

    if (sc)
    {
      sc->removeItem(item);
    }

    item->deleteLater();
  }
}

void deleteGraphicsItem(QGraphicsItem* item)
{
  if (item)
  {
    auto sc = item->scene();

    if (sc)
    {
      sc->removeItem(item);
    }

    delete item;
  }
}

QGraphicsView *getView(QGraphicsItem &self)
{
    if(!self.scene())
        return nullptr;
    auto v = self.scene()->views();
    if(v.empty())
        return nullptr;
    return v.first();
}
