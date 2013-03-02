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
#include "messages/greeter.hpp"
#include "util/dbc.hpp"

namespace m = fire::message;
namespace u = fire::util;

namespace fire
{
    namespace messages
    {
        const std::string GREET_REGISTER = "greet_register";
        const std::string GREET_FIND_REQUEST = "greet_request";
        const std::string GREET_FIND_RESPONSE = "greet_response";

        greet_register::greet_register(
                const std::string& id,
                const greet_endpoint& local,
                const std::string& response_service_address) :
            _id{id},
            _local(local),
            _response_service_address{response_service_address}
        {
        }

        greet_register::greet_register(const m::message& m)
        {
            REQUIRE_EQUAL(m.meta.type, GREET_REGISTER);

            _id = m.meta.extra["from_id"].as_string();
            _local.ip = m.meta.extra["loc_ip"].as_string();
            _local.port = m.meta.extra["loc_port"].as_string();
            _response_service_address = m.meta.extra["response_address"].as_string();
        }

        greet_register::operator message::message() const
        {
            m::message m;
            m.meta.type = GREET_REGISTER;
            m.meta.extra["from_id"] = _id;
            m.meta.extra["loc_ip"] = _local.ip;
            m.meta.extra["loc_port"] = _local.port;
            m.meta.extra["response_address"] = _response_service_address;
            return m;
        }

        const std::string& greet_register::id() const
        {
            return _id;
        }

        const greet_endpoint& greet_register::local() const
        {
            return _local;
        }

        const std::string& greet_register::response_service_address() const
        {
            return _response_service_address;
        }

        greet_find_request::greet_find_request(
                const std::string& from_id,
                const std::string& search_id) :
            _from_id{from_id},
            _search_id{search_id}
        {
        }

        greet_find_request::greet_find_request(const m::message& m)
        {
            REQUIRE_EQUAL(m.meta.type, GREET_FIND_REQUEST);

            _from_id = m.meta.extra["from_id"].as_string();
            _search_id = m.meta.extra["search_id"].as_string();
        }

        greet_find_request::operator message::message() const
        {
            m::message m;
            m.meta.type = GREET_FIND_REQUEST;
            m.meta.extra["from_id"] = _from_id;
            m.meta.extra["search_id"] = _search_id;
            return m;
        }

        const std::string& greet_find_request::from_id() const
        {
            return _from_id;
        }

        const std::string& greet_find_request::search_id() const
        {
            return _search_id;
        }

        greet_find_response::greet_find_response(
                bool found,
                const std::string& id,
                const greet_endpoint& local,
                const greet_endpoint& ext) :
            _found{found},
            _id{id},
            _local(local),
            _ext(ext)
        {
        }

        greet_find_response::greet_find_response(const m::message& m)
        {
            REQUIRE_EQUAL(m.meta.type, GREET_FIND_RESPONSE);

            _id = m.meta.extra["search_id"].as_string();
            _local.ip = m.meta.extra["loc_ip"].as_string();
            _local.port = m.meta.extra["loc_port"].as_string();
            _ext.ip = m.meta.extra["ext_ip"].as_string();
            _ext.port = m.meta.extra["ext_port"].as_string();
            _found = m.meta.extra["found"].as_int() == 1;
        }

        greet_find_response::operator message::message() const
        {
            m::message m;
            m.meta.type = GREET_FIND_RESPONSE;
            m.meta.extra["search_id"] = _id;
            m.meta.extra["loc_ip"] = _local.ip;
            m.meta.extra["loc_port"] = _local.port;
            m.meta.extra["ext_ip"] = _ext.ip;
            m.meta.extra["ext_port"] = _ext.port;
            m.meta.extra["found"] = static_cast<int>(_found ? 1 : 0);
            return m;
        }

        
        bool greet_find_response::found() const
        {
            return _found;
        }
        
        const std::string& greet_find_response::id() const
        {
            return _id;
        }

        const greet_endpoint& greet_find_response::local() const
        {
            return _local;
        }

        const greet_endpoint& greet_find_response::external() const
        {
            return _ext;
        }

    }
}
