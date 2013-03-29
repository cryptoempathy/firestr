/*
 * Copyright (C) 2013  Maxim Noah Khailo
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
#include "gui/debugwin.hpp"

#include "gui/list.hpp"

#include "util/log.hpp"

#include <fstream>

#include <QVBoxLayout>
#include <QTabWidget>
#include <QGridLayout>
#include <QFileInfo>
#include <QTimer>
#include <QSettings>

namespace us = fire::user;
namespace s = fire::session;
namespace m = fire::message;

namespace fire
{
    namespace gui
    {
        namespace 
        {
            const std::size_t UPDATE_GRAPH = 100;
            const std::size_t UPDATE_LOG = 1000;
            const std::size_t UPDATE_MAILBOXES = 1000;
            const std::size_t GRAPH_WIDTH = 500;
            const std::size_t GRAPH_STEP = 2;
        }

        void init_scene(QGraphicsView& v)
        {
            auto s = new QGraphicsScene;
            //s->setSceneRect(v.visibleRegion().boundingRect());
            v.setScene(s);
            ENSURE(v.scene());
        }

        queue_debug::queue_debug(const std::string& name, m::mailbox_stats& stats) :
            _mailbox{},
            _mailbox_stats{&stats},
            _name_s{name},
            _in_max{0},
            _out_max{0},
            _x{0},
            _prev_in_push{0},
            _prev_out_push{0},
            _prev_in_pop{0},
            _prev_out_pop{0}
        {
            REQUIRE(_mailbox_stats);
            _mailbox_stats->on = true;
            init_gui();
        }

        queue_debug::queue_debug(m::mailbox_wptr m) :
            _mailbox{m},
            _mailbox_stats{nullptr},
            _in_max{0},
            _out_max{0},
            _x{0},
            _prev_in_push{0},
            _prev_out_push{0},
            _prev_in_pop{0},
            _prev_out_pop{0}
        {
            auto mb = m.lock();
            if(!mb) return;

            mb->stats(true);
            _name_s = mb->address();

            init_gui();
        }

        void queue_debug::init_gui()
        {
            auto* layout = new QGridLayout{this};
            setLayout(layout);

            _name = new QLabel{_name_s.c_str()};
            layout->addWidget(_name, 0,0,2,1);

            _in_graph = new QGraphicsView;
            _out_graph = new QGraphicsView;

            layout->addWidget(_in_graph, 0,1,1,3);
            layout->addWidget(_out_graph, 1,1,1,3);

            init_scene(*_in_graph);
            init_scene(*_out_graph);

            auto *t = new QTimer(this);
            connect(t, SIGNAL(timeout()), this, SLOT(update_graph()));
            t->start(UPDATE_GRAPH);
        }

        queue_debug::~queue_debug()
        {
            auto mb = _mailbox.lock();
            if(mb) mb->stats(false);
            if(_mailbox_stats) _mailbox_stats->on = false;
        }

        void draw_graph(QGraphicsView& v, int px, int py, int x, int y, size_t& max_y, const QPen& pen)
        {
            REQUIRE(v.scene());
            if(y > max_y) max_y = y;

            auto s = v.scene();
            s->addLine(px, -py, x, -y, pen);

            v.fitInView(x-GRAPH_WIDTH,-int(max_y),GRAPH_WIDTH, max_y+2); 
        }

        void queue_debug::update_graph()
        {
            INVARIANT(_in_graph);
            INVARIANT(_out_graph);

            auto px = _x;
            _x+=GRAPH_STEP;

            m::mailbox_stats* stats = nullptr; 
            
            if(_mailbox_stats) stats = _mailbox_stats;
            else
            {
                auto mb = _mailbox.lock();
                if(!mb) return;
                stats = &(mb->stats());
            }

            CHECK(stats);
            auto in_push = stats->in_push_count;
            auto in_pop = stats->in_pop_count;
            auto out_push = stats->out_push_count;
            auto out_pop = stats->out_pop_count;
            stats->reset();

            draw_graph(*_in_graph, px, _prev_in_push, _x, in_push, _in_max, QPen{QColor{"red"}});
            draw_graph(*_in_graph, px, _prev_in_pop, _x, in_pop, _in_max, QPen{QColor{"orange"}});
            draw_graph(*_out_graph, px, _prev_out_push, _x, out_push, _out_max, QPen{QColor{"green"}});
            draw_graph(*_out_graph, px, _prev_out_pop, _x, out_pop, _out_max, QPen{QColor{"blue"}});
                    
            _prev_in_push = in_push;
            _prev_in_pop = in_pop;
            _prev_out_push = out_push;
            _prev_out_pop = out_pop;
        }

        debug_win::debug_win(
                m::post_office_ptr p,
                us::user_service_ptr us, 
                s::session_service_ptr ss, 
                QWidget* parent) :
            _post{p},
            _user_service{us},
            _session_service{ss},
            _log_last_file_pos{0},
            _total_mailboxes{0},
            QDialog{parent}
        {
            REQUIRE(p);
            REQUIRE(us);
            REQUIRE(ss);

            //main layout
            auto* layout = new QVBoxLayout{this};
            setLayout(layout);
            auto* tabs = new QTabWidget{this};
            layout->addWidget(tabs);

            //create log tab
            auto* log_tab = new QWidget;
            auto* log_layout = new QGridLayout{log_tab};

            _log = new QTextEdit;
            log_layout->addWidget(_log, 0,0);

            //create mailbox tab
            auto* mailbox_tab = new QWidget;
            auto* mailbox_layout = new QGridLayout{mailbox_tab};
            _mailboxes = new list;
            mailbox_layout->addWidget(_mailboxes, 0,0);

            //add tabs
            tabs->addTab(log_tab, "log");
            tabs->addTab(mailbox_tab, "mailboxes");

            //add outside message queue to mailboxes tab
            auto outside = new queue_debug{"outside", _post->outside_stats()};
            _mailboxes->add(outside);

            //fill tabs with data 
            update_mailboxes();
            update_log();

            //setup timers
            auto *t = new QTimer(this);
            connect(t, SIGNAL(timeout()), this, SLOT(update_log()));
            t->start(UPDATE_LOG);

            auto *t2 = new QTimer(this);
            connect(t2, SIGNAL(timeout()), this, SLOT(update_mailboxes()));
            t2->start(UPDATE_MAILBOXES);

            restore_state();

            ENSURE(_log);
            ENSURE(_post);
            ENSURE(_user_service);
            ENSURE(_session_service);
        }

        void add_mailboxes(m::post_office& o, list& l, added_mailboxes& added)
        {
            for(const auto& m : o.boxes())
            {
                if(added.count(m.first)) continue;

                auto mb = new queue_debug{m.second};
                l.add(mb);
                added.insert(m.first);
            }

            for(const auto& of : o.offices())
            {
                auto ofp = of.second.lock();
                if(!ofp) continue;

                add_mailboxes(*ofp, l, added);
            }
        }

        void debug_win::update_mailboxes()
        {
            INVARIANT(_post);
            INVARIANT(_mailboxes);

            if(_post->boxes().size() == _total_mailboxes) return;

            add_mailboxes(*_post, *_mailboxes, _added_mailboxes);

            _total_mailboxes = _post->boxes().size();
        }

        void debug_win::update_log()
        {
            INVARIANT(_log);

            const auto& log_file = LOG_PATH;
            auto modified = QFileInfo(log_file.c_str()).lastModified();
            if(_log_last_file_pos != 0 && _log_last_modified == modified) return;

            std::ifstream i(log_file.c_str());
            if(!i.good()) return;

            i.seekg(_log_last_file_pos);

            std::string l;
            while(std::getline(i, l))
            {
                std::cerr << l << std::endl;
                _log->append(l.c_str());
                _log_last_file_pos = i.tellg();
            }

            _log_last_modified = modified;
        }

        void debug_win::closeEvent(QCloseEvent *event)
        {
            save_state();
        }

        void debug_win::save_state()
        {
            QSettings settings("mempko", "firestr");
            settings.setValue("debug_win/geometry", saveGeometry());
        }

        void debug_win::restore_state()
        {
            QSettings settings("mempko", "firestr");
            restoreGeometry(settings.value("debug_win/geometry").toByteArray());
        }
    }
}