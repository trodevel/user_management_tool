/*

User Management Tool.

Copyright (C) 2018 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 9330 $ $Date:: 2018-06-08 #$ $Author: serge $

#include <iostream>
#include <string>

#include "user_manager/user_manager.h"              // user_manager::UserManager
#include "password_hasher/login_to_id_converter.h"  // password_hasher::convert_login_to_id
#include "utils/to_value.h"                         // utils::to_value
#include "utils/mutex_helper.h"                     // MUTEX_SCOPE_LOCK

void print_help()
{
    std::cout
            << "USAGE: user_manager_tools <command> <param_1> [<param_2> ...]\n"
            << "\n"
            << "where:\n"
            << "<command>   - one of init, add, delete, update\n"
            << "\n"
            << "command init:\n"
            << "\n"
            << "user_manager_tools init <users_dat>\n"
            << "\n"
            << "<users_dat> - file with users\n"
            << "\n"
            << "command add (or a):\n"
            << "\n"
            << "user_manager_tools add <users_dat> <group_id> <status> <login> <password> <gender> <name> <first_name> <company_name> <email> <phone> <timezone>\n"
            << "\n"
            << "<users_dat> - file with users\n"
            << "\n"
            << "command delete (or d):\n"
            << "\n"
            << "user_manager_tools delete <users_dat> <login>\n"
            << "\n"
            << "<users_dat> - file with users\n"
            << "<login>     - user login\n"
            << "\n"
            << "command update (or u):\n"
            << "\n"
            << "user_manager_tools update <users_dat> <login> <field> <value>\n"
            << "\n"
            << "<users_dat> - file with users\n"
            << "<login>     - user login\n"
            << "<field>     - field name to update\n"
            << "<value>     - new value\n"
            << "\n"
            << "\n";
}

user_manager::User * create_user(
        const std::string & group_id,
        const std::string & status,
        const std::string & login,
        const std::string & password,
        const std::string & gender,
        const std::string & name,
        const std::string & first_name,
        const std::string & company_name,
        const std::string & email,
        const std::string & phone,
        const std::string & timezone )
{
    auto res = new user_manager::User;

    res->user_id        = password_hasher::convert_login_to_id( login );
    utils::to_value( & res->group_id, group_id );
    res->status         = std::stoi( status ) ? user_manager::status_e::ACTIVE : user_manager::status_e::INACTIVE;
    res->login          = login;
    res->password_hash  = password_hasher::convert_password_to_hash( password );;

    res->gender         = std::stoi( gender ) == 1 ? user_manager::gender_e::MALE : user_manager::gender_e::FEMALE;
    res->name           = name;
    res->first_name     = first_name;
    res->company_name   = company_name;
    res->email          = email;
    //res->email_2;
    res->phone          = phone;
    //res->phone_2        = ;
    res->timezone       = timezone;

    return res;
}

int init_file(
        const std::string & filename )
{
    user_manager::UserManager m;

    auto b = m.init( filename );

    if( b )
    {
        std::cout << "ERROR: file already exists " << filename << std::endl;
        return EXIT_FAILURE;
    }

    std::string error_msg;

    b = m.save( & error_msg, filename );

    if( b == false )
    {
        std::cout << "ERROR: cannot write file: " << error_msg << std::endl;
    }

    std::cout << "OK: created user file " << filename << std::endl;

    return EXIT_SUCCESS;
}

int add_user(
        const std::string & filename,
        const std::string & group_id,
        const std::string & status,
        const std::string & login,
        const std::string & password,
        const std::string & gender,
        const std::string & name,
        const std::string & first_name,
        const std::string & company_name,
        const std::string & email,
        const std::string & phone,
        const std::string & timezone )
{
    user_manager::UserManager m;

    auto b = m.init( filename );

    if( b == false)
    {
        std::cout << "ERROR: cannot load users from " << filename << std::endl;
        return EXIT_FAILURE;
    }

    auto user = create_user( group_id, status, login, password, gender, name, first_name, company_name, email, phone, timezone );

    std::string error_msg;

    b = m.add( user, error_msg );

    if( b == false )
    {
        std::cout << "ERROR: cannot add user - " << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "OK: user was added, user_id " << user->user_id << std::endl;

    b = m.save( & error_msg, filename );

    if( b == false )
    {
        std::cout << "ERROR: cannot write file: " << error_msg << std::endl;
    }

    std::cout << "OK: user file was written" << std::endl;

    return EXIT_SUCCESS;
}

int delete_user(
        const std::string & filename,
        const std::string & login )
{
    user_manager::UserManager m;

    auto b = m.init( filename );

    if( b == false)
    {
        std::cout << "ERROR: cannot load users from " << filename << std::endl;
        return EXIT_FAILURE;
    }

    auto user = m.find( login );

    if( user == nullptr )
    {
        std::cout << "ERROR: cannot find user " << login << std::endl;
        return EXIT_FAILURE;
    }

    auto user_id = user->user_id;

    std::string error_msg;

    b = m.delete_user( user_id, & error_msg );

    if( b == false )
    {
        std::cout << "ERROR: cannot delete user - " << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "OK: user was deleted, user_id " << user_id << std::endl;

    b = m.save( & error_msg, filename );

    if( b == false )
    {
        std::cout << "ERROR: cannot write file: " << error_msg << std::endl;
    }

    std::cout << "OK: user file was written" << std::endl;

    return EXIT_SUCCESS;
}

int update(
        const std::string & filename,
        const std::string & login,
        const std::string & field,
        const std::string & value )
{
    user_manager::UserManager m;

    auto b = m.init( filename );

    if( b == false)
    {
        std::cout << "ERROR: cannot load users from " << filename << std::endl;
        return EXIT_FAILURE;
    }

    {
        auto & mutex = m.get_mutex();

        MUTEX_SCOPE_LOCK( mutex );

        auto user = m.find( login );

        if( user == nullptr )
        {
            std::cout << "ERROR: cannot find user " << login << std::endl;
            return EXIT_FAILURE;
        }

        if( field == "status" )
        {
            user->status    = std::stoi( value ) ? user_manager::status_e::ACTIVE : user_manager::status_e::INACTIVE;
        }
        else if( field == "name" )
        {
            user->name          = value;
        }
        else if( field == "first_name" )
        {
            user->first_name    = value;
        }
        else if( field == "company_name" )
        {
            user->company_name  = value;
        }
        else if( field == "password" )
        {
            user->password_hash = password_hasher::convert_password_to_hash( value );
        }
        else if( field == "timezone" )
        {
            user->timezone      = value;
        }
        else if( field == "gender" )
        {
            user->gender        = std::stoi( value ) == 1 ? user_manager::gender_e::MALE : user_manager::gender_e::FEMALE;
        }
        else
        {
            std::cout << "ERROR: unknown or read-only field '" << field << "'" << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "OK: field '" << field << "' was updated" << std::endl;

    std::string error_msg;

    b = m.save( & error_msg, filename );

    if( b == false )
    {
        std::cout << "ERROR: cannot write file: " << error_msg << std::endl;
    }

    std::cout << "OK: user file was written" << std::endl;

    return EXIT_SUCCESS;
}

int main( int argc, const char* argv[] )
{
    if( argc <= 1 )
    {
        std::cout << "ERROR: command is not given" << std::endl;
        return EXIT_FAILURE;
    }

    std::string command = argv[1];

    if( command == "-h" || command == "--help" )
    {
        print_help();
        return EXIT_SUCCESS;
    }
    else if( command == "init" || command == "i" )
    {
        const int expected = 1;
        if( argc < expected + 2 )
        {
            std::cout << "ERROR: not enough arguments for command, given " << argc - 2 << ",  expected " << expected << std::endl;
            return EXIT_FAILURE;
        }

        return init_file( argv[2] );
    }
    else if( command == "add" || command == "a" )
    {
        const int expected = 12;
        if( argc < expected + 2 )
        {
            std::cout << "ERROR: not enough arguments for command, given " << argc - 2 << ",  expected " << expected << std::endl;
            return EXIT_FAILURE;
        }

        return add_user( argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12], argv[13] );
    }
    else if( command == "delete" || command == "d" )
    {
        const int expected = 2;
        if( argc < expected + 2 )
        {
            std::cout << "ERROR: not enough arguments for command, given " << argc - 2 << ",  expected 2" << std::endl;
            return EXIT_FAILURE;
        }

        return delete_user( argv[2], argv[3] );
    }
    else if( command == "update" || command == "u" )
    {
        const int expected = 4;
        if( argc < expected + 2 )
        {
            std::cout << "ERROR: not enough arguments for command, given " << argc - 2 << ",  expected " << expected << std::endl;
            return EXIT_FAILURE;
        }

        return update( argv[2], argv[3], argv[4], argv[5] );
    }

    std::cout << "ERROR: unknown command " << command << std::endl;

    return EXIT_FAILURE;
}
