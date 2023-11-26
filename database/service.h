#ifndef AUTHOR_H
#define AUTHOR_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class Service{
        private:
            long        _id;
            std::string _name;
            std::string _type;
            std::string _desc;
            std::string _price;
            std::string _date;
            long        _author_id;

        public:

            const long        &get_id() const;
            const std::string &get_name() const;
            const std::string &get_type() const;
            const std::string &get_desc() const;
            const std::string &get_price() const;
            const std::string &get_date() const;
            const long        &get_author_id() const;

            long        &id();
            std::string &name();
            std::string &type();
            std::string &desc();
            std::string &price();
            std::string &date();
            long        &author_id();

            bool is_user_exist();

            static void init();
            bool        update_in_mysql();
            bool        save_to_mysql();
            static bool delete_in_mysql(long id);

            static std::optional<Service>   read_by_id(long id);
            static std::vector<Service>     read_all();
            static std::vector<Service>     search(std::string name,std::string type);

            static Service          fromJSON(const std::string & str);
            Poco::JSON::Object::Ptr toJSON() const;
    };
}

#endif