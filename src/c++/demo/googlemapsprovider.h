//
// Created by whs31 on 19.09.23.
//

#pragma once
#include <QtCore/QObject>

namespace CCL
{
  class GoogleMapsProvider : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(QString path READ path CONSTANT STORED false FINAL)

    public:
      explicit GoogleMapsProvider(QObject* parent = nullptr);
      [[nodiscard]] static QString path() ;

    private:
      void create() noexcept;
  };
} // CCL
