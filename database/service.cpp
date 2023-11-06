#include "service.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
#define DATE_FORMAT "\'DD.MM.YYYY HH24:MI:SS\'"

namespace database
{
    void Service::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `service` ("
                        << "`service_id` INT NOT NULL AUTO_INCREMENT,"
                        << "`name` VARCHAR(256) NOT NULL,"
                        << "`type` VARCHAR(40) NOT NULL,"
                        << "`desc` VARCHAR(4000),"
                        << "`price` DECIMAL(19, 4) NOT NULL,"
                        << "`date` DATETIME NOT NULL,"
                        << "`author_id` INT NOT NULL,"
                        << "PRIMARY KEY (`service_id`),"
                        << "FOREIGN KEY (`author_id`) REFERENCES `user` (`user_id`)"
                        << ");",
                now;
            
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
    }

    bool Service::update_in_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement update(session);

            update  << "UPDATE `service` "
                    << "SET `name` = ?, `type` = ?, `desc` = ?, `price` = ?, `date` = NOW(), `author_id`= ? "
                    << "WHERE `service_id` = ? ; ",
                use(_name),
                use(_type),
                use(_desc),
                use(_price),
                use(_author_id),
                use(_id),
                now;

            update.execute();

            std::cout << "updated: " << _id << std::endl;
            return true;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return false;
    }

    bool Service::delete_in_mysql(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement del(session);
            
            Service a;
            del << "DELETE FROM `service` WHERE `service_id` = ?;",
                use(id),
                range(0, 1); 
            del.execute();
            std::cout << "deleted: " << id << std::endl;
            return true;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return false;
    }

    Poco::JSON::Object::Ptr Service::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("name", _name);
        root->set("type", _type);
        root->set("desc", _desc);
        root->set("price", _price);
        root->set("date", _date);
        root->set("author_id", _author_id);

        return root;
    }

    Service Service::fromJSON(const std::string &str)
    {
        Service user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.name() = object->getValue<std::string>("name");
        user.type() = object->getValue<std::string>("type");
        user.desc() = object->getValue<std::string>("desc");
        user.price() = object->getValue<std::string>("price");
        user.date() = object->getValue<std::string>("date");
        user.author_id() = object->getValue<long>("author_id");

        return user;
    }

    std::optional<Service> Service::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            Service a;
            select  << "SELECT `service_id`, `name`, `type`, `desc`, CAST(`price` as CHAR(25)), TO_CHAR(`date`, " << DATE_FORMAT << "), `author_id` "
                    << "FROM `service` " 
                    << "WHERE `service_id` = ? ; ",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._desc),
                into(a._price),
                into(a._date),
                into(a._author_id),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return {};
    }

    std::vector<Service> Service::read_all()
    {
        std::vector<Service> result;
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            Service a;
            select  << "SELECT `service_id`, `name`, `type`, `desc`, CAST(`price` as CHAR(25)), TO_CHAR(`date`, " << DATE_FORMAT << "), `author_id` " 
                    << "FROM `service` ; ",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._desc),
                into(a._price),
                into(a._date),
                into(a._author_id),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return std::vector<Service>();
    }

    std::vector<Service> Service::search(std::string name, std::string type)
    {
        std::vector<Service> result;

        std::transform(name.begin(), name.end(), name.begin(),
        [](unsigned char c){ return std::tolower(c); });

        std::transform(type.begin(), type.end(), type.begin(),
        [](unsigned char c){ return std::tolower(c); });

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            Service a;
            name += "%";
            type += "%";
            select  << "SELECT `service_id`, `name`, `type`, `desc`, CAST(`price` as CHAR(25)), TO_CHAR(`date`, "<< DATE_FORMAT << "), `author_id`" 
                    << "FROM service where LOWER(`name`) LIKE ? and LOWER(`type`) LIKE ?",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._desc),
                into(a._price),
                into(a._date),
                into(a._author_id),
                use(name),
                use(type),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return std::vector<Service>();
    }

    bool Service::save_to_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert  << "INSERT INTO `service` (`name`, `type`, `desc`, `price`, `date`, `author_id` ) "
                    << "VALUES( ?, ?, ?, ?, NOW(), ? ) ; ",
                use(_name),
                use(_type),
                use(_desc),
                use(_price),
                use(_author_id);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted: " << _id << std::endl;
            return true;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.displayText() << std::endl;
        }
        return false;
    }

    long &Service::id()
    {
        return _id;
    }

    std::string &Service::name()
    {
        return _name;
    }

    std::string &Service::type()
    {
        return _type;
    }

    std::string &Service::desc()
    {
        return _desc;
    }

    std::string &Service::price()
    {
        return _price;
    }

    std::string &Service::date()
    {
        return _date;
    }

    long &Service::author_id()
    {
        return _author_id;
    }

    const long &Service::get_id() const
    {
        return _id;
    }

    const std::string &Service::get_name() const
    {
        return _name;
    }

    const std::string &Service::get_type() const
    {
        return _type;
    }

    const std::string &Service::get_desc() const
    {
        return _desc;
    }

    const std::string &Service::get_price() const
    {
        return _price;
    }

    const std::string &Service::get_date() const
    {
        return _date;
    }

    const long &Service::get_author_id() const
    {
        return _author_id;
    }

}