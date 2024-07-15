#include "../libft.h"

// ft_isprint()
// checks for any printable character including space.
int ft_isprint(int c)
{
    return (32 <= c && c <= 126);
}

// what is a printable character?

// A printable character is a character that has a graphical representation.

// The following characters are considered printable:

// 1. Alphabetic characters: a-z, A-Z
// 2. Digits: 0-9
// 3. Punctuation: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
// 4. Space: ' '

// The following characters are not considered printable:

// 1. Control characters: characters that are not printable and are used to control devices (e.g., newline, tab, backspace)
// 2. Non-printable characters: characters that do not have a graphical representation (e.g., null character, bell character)
