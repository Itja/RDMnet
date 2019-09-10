/******************************************************************************
 * Copyright 2019 ETC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************
 * This file is a part of RDMnet. For more information, go to:
 * https://github.com/ETCLabs/RDMnet
 *****************************************************************************/

#pragma once

#include "ControllerUtils.h"

BEGIN_INCLUDE_QT_HEADERS()
#include <QStyledItemDelegate>
END_INCLUDE_QT_HEADERS()

class PropertyEditorsDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit PropertyEditorsDelegate(QObject* parent = 0);
  ~PropertyEditorsDelegate();

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;
};
