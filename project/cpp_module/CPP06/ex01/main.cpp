#include <iostream>
#include "Serializer.hpp"

int main() {
    Data original;
    original.id = 123;
    original.name = "Zombie";
    original.value = 42.42f;

    // Serialize
    unsigned long raw = Serializer::serialize(&original);
    std::cout << "Serialized value: " << raw << std::endl;

    // Deserialize
    Data* result = Serializer::deserialize(raw);

    if (result == &original)
        std::cout << "Pointer match: deserialize(serialize(ptr)) == ptr" << std::endl;
    else
        std::cout << "Pointer mismatch!" << std::endl;

    std::cout << "id: " << result->id << std::endl;
    std::cout << "name: " << result->name << std::endl;
    std::cout << "value: " << result->value << std::endl;

    return 0;
}

