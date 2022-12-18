#include <iostream>
#include "mcpkg/utils/database.h"
#include "toml++/toml.h"

bool greater(std::string, std::string);

int main() {
    Database db{};
    db.open("pkgs");
    auto result = db.exec("SELECT * FROM package");
    for (auto row:result.data.data) {
        for (auto pair:row) {
            std::cout<<pair.first<< ":" <<pair.second<<std::endl;
        }
        std::cout<<(row["pkg_name"]);
    }
}
