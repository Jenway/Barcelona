#ifndef AMATERIA_HPP
#define AMATERIA_HPP

#include <string>
#include <iostream>

class ICharacter;  // 前向声明

class AMateria
{
protected:
    std::string type;

public:
    AMateria(std::string const & type);
    AMateria(const AMateria& other);
    AMateria& operator=(const AMateria& other);
    virtual ~AMateria();

    std::string const & getType() const;

    virtual AMateria* clone() const = 0;
    virtual void use(ICharacter& target);
};

#endif

