/*
 * Copyright (C) 2012  Maxim Noah Khailo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIRESTR_GUI_LIST_H
#define FIRESTR_GUI_LIST_H

#include <QScrollArea>
#include <QVBoxLayout>

namespace fire
{
    namespace gui
    {
        class list : public QScrollArea
        {
            Q_OBJECT
            public:
                list(const std::string& name);

            public slots:
                virtual void add(QWidget*);
                virtual void clear();

            protected:
                const std::string _name;
                QWidget* _root;
                QVBoxLayout* _layout; 
        };
    }
}

#endif