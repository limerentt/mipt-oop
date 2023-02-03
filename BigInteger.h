#include <iostream>
#include <vector>
#include <string>

const int base = 1e9;

class BigInteger;

BigInteger operator+(const BigInteger &first, const BigInteger &second);

BigInteger operator-(const BigInteger &first, const BigInteger &second);

BigInteger operator*(const BigInteger &first, const BigInteger &second);

BigInteger operator/(const BigInteger &first, const BigInteger &second);

BigInteger operator%(const BigInteger &first, const BigInteger &second);

bool operator!=(const BigInteger &twin1, const BigInteger &twin2);

bool operator<=(const BigInteger &twin1, const BigInteger &twin2);

bool operator>=(const BigInteger &twin1, const BigInteger &twin2);

class BigInteger {
    mutable bool sign;
    std::vector<long long> num;
public:
    BigInteger() {
        sign = true;
        num = {0};
    }

    BigInteger(const BigInteger &number) {
        num = number.num;
        sign = number.sign;
    }

    ~BigInteger() = default;

    BigInteger(long long value) {
        if (value > 0) {
            sign = true;
            if (value >= base) {
                num.push_back(value % base);
                num.push_back(value / base);
            } else {
                num.push_back(value);
            }
        } else {
            if (value == 0) {
                sign = true;
                num.push_back(0);
            } else {
                sign = false;
                value = -value;
                if (value >= base) {
                    num.push_back(value % base);
                    num.push_back(value / base);
                } else {
                    num.push_back(value);
                }
            }
        }
    }

    BigInteger(const std::string &str) {
        if (str == "-0" || str == "0") {
            sign = true;
            num.push_back(0);
            return;
        }
        if (str[0] == '-') {
            sign = false;
            for (long long i = static_cast<long long>(str.size()) - 9; i >= 1; i -= 9) {
                num.push_back(std::stoi(str.substr(i, 9)));
            }
            if (str.size() % 9 != 1)
                num.push_back(std::stoi(str.substr(1, (str.size() % 9 + 8) % 9)));
        } else {
            sign = true;
            for (long long i = static_cast<long long>(str.size()) - 9; i >= 1; i -= 9) {
                num.push_back(std::stoi(str.substr(i, 9)));
            }
            if (str.size() % 9 != 0)
                num.push_back(std::stoi(str.substr(0, str.size() % 9)));
        }
    }

    explicit operator bool() const {
        return *this != 0;
    }

    BigInteger &operator=(const BigInteger &number) {
        BigInteger copy(number);
        swap(copy);
        return *this;
    }

    void swap(BigInteger &twin) {
        std::swap(sign, twin.sign);
        std::swap(num, twin.num);
    }

    void remove_zeros() {
        while (num.size() > 1 && num[num.size() - 1] == 0) {
            num.pop_back();
        }
    }

    void shift() {
        num.resize(num.size() + 1);
        for (long long i = num.size() - 1; i >= 1; --i) {
            num[i] = num[i - 1];
        }
        num[0] = 0;
    }

    BigInteger abs() const {
        BigInteger copy = *this;
        if (!copy.sign)
            copy.sign = true;
        return copy;
    }

    BigInteger &sum(const BigInteger &number) {
        for (size_t i = 0; i < number.num.size(); ++i) {
            if (i > num.size() - 1) {
                num.push_back(number.num[i]);
            } else {
                if (num[i] + number.num[i] >= base) {
                    if (i == num.size() - 1)
                        num.push_back((num[i] + number.num[i]) / base);
                    else
                        num[i + 1] += (num[i] + number.num[i]) / base;
                }
                num[i] = (num[i] + number.num[i]) % base;
            }
        }
        return *this;
    }

    BigInteger &positive_difference(const BigInteger &number) {
        for (size_t i = 0; i < num.size(); ++i) {
            if (i >= number.num.size()) {
                break;
            }
            if (num[i] >= number.num[i]) {
                num[i] -= number.num[i];
            } else {
                --num[i + 1];
                num[i] += base - number.num[i];
            }
        }
        for (size_t i = 0; i < num.size() - 1; ++i) {
            if (num[i] < 0) {
                --num[i + 1];
                num[i] += base;
            }
        }
        remove_zeros();
        return *this;
    }

    BigInteger &negative_difference(const BigInteger &number) {
        sign = !sign;
        size_t old_size = num.size();
        num.resize(number.num.size() + 1);
        for (size_t i = old_size; i < num.size() - 1; ++i) {
            num[i] = 0;
        }
        num[num.size() - 1] = 1;
        positive_difference(number);

        size_t index = 0;
        while (num[index] == 0)
            ++index;

        num[index] = base - num[index];
        for (size_t i = index + 1; i < num.size(); ++i) {
            num[i] = base - 1 - num[i];
        }

        remove_zeros();
        return *this;
    }

    BigInteger &operator+=(const BigInteger &number) {
        if (this == &number) {
            BigInteger copy;
            copy = number;
            return *this += copy;
        }
        if (sign == number.sign) {
            sum(number);
            return *this;
        }
        if (abs_equal(*this, number)) {
            num.assign(1, 0);
            sign = true;
            return *this;
        }
        if (abs_greater(*this, number)) {
            positive_difference(number);
            return *this;
        }
        negative_difference(number);
        return *this;
    }

    BigInteger &operator-=(const BigInteger &number) {
        if (this == &number) {
            BigInteger copy;
            copy = number;
            return *this -= copy;
        }
        if (number == 0) {
            return *this;
        }
        number.sign = !number.sign;
        *this += number;
        number.sign = !number.sign;
        return *this;
    }

    BigInteger &operator*=(const BigInteger &number) {
        BigInteger res;
        res.sign = (sign == number.sign);

        if (*this == 0 || number == 0) {
            sign = true;
            num.assign(1, 0);
            return *this;
        }

        res.num.assign(num.size() + number.num.size() - 1, 0);

        for (size_t i = 0; i < num.size(); ++i) {
            for (size_t j = 0; j < number.num.size(); ++j) {
                res.num[i + j] += (num[i] * number.num[j]);
                if (res.num[i + j] >= base) {
                    if (i + j >= res.num.size() - 1)
                        res.num.push_back(res.num[i + j]/base);
                    else
                        res.num[i + j + 1] += res.num[i + j]/base;
                    res.num[i + j] %= base;
                }
            }
        }

        for (size_t i = 0; i < res.num.size() - 1; ++i) {
            if (res.num[i] >= base) {
                res.num[i + 1] += (res.num[i] / base);
                res.num[i] %= base;
            }
        }
        res.remove_zeros();
        if (res.num[res.num.size() - 1] >= base) {
            res.num.push_back(res.num[res.num.size() - 1] / base);
            res.num[res.num.size() - 2] %= base;
        }
        *this = res;
        remove_zeros();
        return *this;
    }

    BigInteger &operator/=(const BigInteger& number) {
        if (this == &number) {
            BigInteger copy;
            copy = number;
            return *this /= copy;
        }
        BigInteger quotient;
        BigInteger present_dividend;

        if (sign == number.sign)
            sign = true;
        else
            sign = false;
        if (*this == 0)
            return *this;

        bool changed = false;
        if (!number.sign) {
            number.sign = true;
            changed = true;
        }

        quotient.num.assign(num.size(), 0);
        if (abs_greater(number, *this)) {
            sign = true;
            num.assign(1, 0);
            return *this;
        }
        size_t left, right, mid;

        BigInteger copy;
        bool lesser = false;
        for (long long i = static_cast<long long>(num.size()) - 1; i >= 0; --i) {
            present_dividend.shift();
            present_dividend.num[0] = num[i];
            present_dividend.remove_zeros();
            left = 0;
            right = base;
            while (left + 1 < right) {
                copy = number;
                mid = (left + right) / 2;
                copy *= mid;
                if (copy == present_dividend) {
                    left = mid;
                    break;
                }
                if (abs_greater(present_dividend, copy) || abs_equal(present_dividend, copy))
                    left = mid + 1;
                else
                    right = mid;
            }
            copy = number;
            copy *= left;
            if (abs_greater(copy, present_dividend)) {
                --left;
                lesser = true;
            }
            if (lesser) {
                present_dividend += number;
                lesser = false;
            }
            present_dividend -= copy;
            present_dividend.remove_zeros();
            quotient.num[i] = left;
        }
        if (changed)
            number.sign = false;
        num = quotient.num;
        remove_zeros();
        return *this;
    }

    BigInteger &operator%=(const BigInteger& number) {
        if (this == &number) {
            BigInteger copy;
            copy = number;
            return *this %= copy;
        }
        BigInteger present_dividend;

        sign = (sign == number.sign);

        bool changed = false;
        if (!number.sign) {
            number.sign = true;
            changed = true;
        }

        if (abs_greater(number, *this)) {
            return *this;
        }

        long long left, right, mid;

        BigInteger copy;
        bool lesser = false;
        for (long long i = static_cast<long long>(num.size()) - 1; i >= 0; --i) {
            present_dividend.shift();
            present_dividend.num[0] = num[i];
            present_dividend.remove_zeros();
            left = 0;
            right = base;
            while (left + 1 < right) {
                copy = number;
                mid = (left + right) / 2;
                copy *= mid;
                if (copy == present_dividend) {
                    left = mid;
                    break;
                }
                if (abs_greater(present_dividend, copy) || abs_equal(present_dividend, copy))
                    left = mid + 1;
                else
                    right = mid;
            }
            copy = number;
            copy *= left;
            if (abs_greater(copy, present_dividend)) {
                --left;
                lesser = true;
            }
            if (lesser) {
                present_dividend += number;
                lesser = false;
            }
            present_dividend -= copy;
            present_dividend.remove_zeros();
        }

        if (changed)
            number.sign = false;
        num = present_dividend.num;
        if (num.size() == 1 && num[0] == 0)
            sign = true;
        remove_zeros();
        return *this;
    }

    std::string toString() const {
        std::string str;
        if (num.size() == 1 && num[0] == 0)
            sign = true;
        if (!sign)
            str += "-";
        str += std::to_string(num[num.size() - 1]);
        size_t num_of_zeroes;
        for (long long i = static_cast<long long>(num.size()) - 2; i >= 0; --i) {
            num_of_zeroes = 9 - std::to_string(num[i]).size();
            for (size_t j = 0; j < num_of_zeroes; ++j) {
                str += "0";
            }
            str += std::to_string(num[i]);
        }
        return str;
    }

    bool operator==(const BigInteger &twin) const {
        if (sign != twin.sign)
            return false;
        if (num.size() != twin.num.size())
            return false;
        for (size_t i = 0; i < num.size(); ++i) {
            if (num[i] != twin.num[i])
                return false;
        }
        return true;
    }

    bool operator>(const BigInteger &twin) const {
        if (sign && !twin.sign)
            return true;
        if (!sign && twin.sign)
            return false;
        if (sign) {
            if (num.size() != twin.num.size()) return (num.size() > twin.num.size());
            for (size_t i = 0; i < num.size(); ++i) {
                if (num[i] != twin.num[i]) return (num[i] > twin.num[i]);
            }
        }
        if (!sign) {
            if (num.size() != twin.num.size()) return (num.size() < twin.num.size());
            for (size_t i = 0; i < num.size(); ++i) {
                if (num[i] != twin.num[i]) return (num[i] < twin.num[i]);
            }
        }
        return false;
    }

    bool operator<(const BigInteger &twin) const {
        if (sign && !twin.sign)
            return false;
        if (!sign && twin.sign)
            return true;
        if (sign) {
            if (num.size() != twin.num.size()) return (num.size() < twin.num.size());
            for (size_t i = 0; i < num.size(); ++i) {
                if (num[i] != twin.num[i]) return (num[i] < twin.num[i]);
            }
        }
        if (!sign) {
            if (num.size() != twin.num.size()) return (num.size() > twin.num.size());
            for (size_t i = 0; i < num.size(); ++i) {
                if (num[i] != twin.num[i]) return (num[i] > twin.num[i]);
            }
        }
        return false;
    }

    BigInteger operator-() const {
        BigInteger res = *this;
        if (*this == 0)
            return res;
        res.sign = !sign;
        return res;
    }

    BigInteger &operator--() {
        return *this -= 1;
    }

    BigInteger &operator++() {
        return *this += 1;
    }

    BigInteger operator--(int) {
        BigInteger copy = *this;
        *this -= 1;
        return copy;
    }

    BigInteger operator++(int) {
        BigInteger copy = *this;
        *this += 1;
        return copy;
    }

    friend std::istream &operator>>(std::istream &stream, BigInteger &num);

    bool abs_equal(const BigInteger &number1, const BigInteger &number2);

    bool abs_greater(const BigInteger &number1, const BigInteger &number2);

    BigInteger gcd(BigInteger twin2) const {
        std::cerr << "GCDGCDGDC" << std::endl;
        BigInteger res;
        res = *this;
        if (!res.sign) {
            res.sign = true;
        }
        if (!twin2.sign) {
            twin2.sign = true;
        }

        while (twin2 != 0) {
            res %= twin2;
            res.swap(twin2);
        }
        return res;
    }
};

bool BigInteger::abs_equal(const BigInteger &number1, const BigInteger &number2) {
    return number1.num == number2.num;
}

bool BigInteger::abs_greater(const BigInteger &number1, const BigInteger &number2) {
    if (number1.num.size() > number2.num.size())
        return true;
    if (number1.num.size() < number2.num.size())
        return false;
    for (long long i = static_cast<long long>(number1.num.size()) - 1; i >= 0; --i) {
        if (number1.num[i] > number2.num[i])
            return true;
        if (number1.num[i] < number2.num[i])
            return false;
    }
    return false;
}

BigInteger operator+(const BigInteger &first, const BigInteger &second) {
    BigInteger copy = first;
    copy += second;
    return copy;
}

BigInteger operator-(const BigInteger &first, const BigInteger &second) {
    BigInteger copy = first;
    copy -= second;
    return copy;
}

BigInteger operator*(const BigInteger &first, const BigInteger &second) {
    BigInteger copy = first;
    copy *= second;
    return copy;
}

BigInteger operator/(const BigInteger &first, const BigInteger &second) {
    BigInteger copy = first;
    copy /= second;
    return copy;
}

BigInteger operator%(const BigInteger &first, const BigInteger &second) {
    BigInteger copy = first;
    copy %= second;
    return copy;
}

std::istream &operator>>(std::istream &stream, BigInteger &number) {
    std::string str;
    stream >> str;
    number = str;
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BigInteger &number) {
    stream << number.toString();
    return stream;
}

bool operator!=(const BigInteger &twin1, const BigInteger &twin2) {
    return !(twin1 == twin2);
}

bool operator<=(const BigInteger &twin1, const BigInteger &twin2) {
    return !(twin1 > twin2);
}

bool operator>=(const BigInteger &twin1, const BigInteger &twin2) {
    return !(twin1 < twin2);
}

class Rational;

Rational operator+(const Rational &first, const Rational &second);

Rational operator-(const Rational &first, const Rational &second);

Rational operator*(const Rational &first, const Rational &second);

Rational operator/(const Rational &first, const Rational &second);

bool operator!=(const Rational &twin1, const Rational &twin2);

bool operator<=(const Rational &twin1, const Rational &twin2);

bool operator>=(const Rational &twin1, const Rational &twin2);

class Rational {
    BigInteger numerator;
    BigInteger denominator = 1;
public:
    Rational() = default;

    Rational(const BigInteger &number) {
        numerator = number;
        denominator = 1;
    }

    Rational(long long number) {
        numerator = number;
        denominator = 1;
    }

    Rational(const BigInteger &first, const BigInteger &second) {
        numerator = first;
        denominator = second;
    }

    void fix() {
        BigInteger mutual = numerator.gcd(denominator);
        numerator /= mutual;
        denominator /= mutual;
        if (denominator < 0) {
            numerator *= (-1);
            denominator *= (-1);
        }
    }

    Rational &operator+=(const Rational &number) {
        numerator *= number.denominator;
        numerator += (number.numerator * denominator);
        denominator *= number.denominator;
        fix();
        return *this;
    }

    Rational &operator-=(const Rational &number) {
        numerator *= number.denominator;
        numerator -= (number.numerator * denominator);
        denominator *= number.denominator;
        fix();
        return *this;
    }

    Rational &operator*=(const Rational &number) {
        numerator *= number.numerator;
        denominator *= number.denominator;
        fix();
        return *this;
    }

    Rational &operator/=(const Rational &number) {
        numerator *= number.denominator;
        denominator *= number.numerator;
        fix();
        return *this;
    }

    Rational operator-() const {
        Rational res = *this;
        if (*this == 0)
            return res;
        res.numerator *= (-1);
        return res;
    }

    Rational &operator=(const Rational &twin) = default;

    bool operator==(const Rational &twin) const {
        if (numerator != twin.numerator)
            return false;
        if (denominator != twin.denominator)
            return false;
        return true;
    }

    bool operator>(const Rational &twin) const {
        if (numerator >= 0 && twin.numerator < 0)
            return true;
        if (numerator <= 0 && twin.numerator > 0)
            return false;
        if (twin.denominator < 0) {
            if (denominator < 0)
                return numerator * twin.denominator > twin.numerator * denominator;
            return numerator * twin.denominator < twin.numerator * denominator;
        }
        if (denominator < 0)
            return numerator * twin.denominator < twin.numerator * denominator;
        return numerator * twin.denominator > twin.numerator * denominator;
    }

    bool operator<(const Rational &twin) const {
        if (numerator <= 0 && twin.numerator > 0)
            return true;
        if (numerator >= 0 && twin.numerator < 0)
            return false;
        if (twin.denominator < 0) {
            if (denominator < 0)
                return numerator * twin.denominator < twin.numerator * denominator;
            return numerator * twin.denominator > twin.numerator * denominator;
        }
        if (denominator < 0)
            return numerator * twin.denominator > twin.numerator * denominator;
        return numerator * twin.denominator < twin.numerator * denominator;
    }

    std::string toString() const {
        if (denominator == 1)
            return (numerator).toString();
        std::string str;

        if (numerator < 0)
            str += '-';
        str += numerator.abs().toString();
        str += '/';
        str += denominator.toString();
        return str;
    }

    std::string asDecimal(size_t precision = 0) const {
        Rational copy = *this;
        std::string res;
        std::string no_coma;
        BigInteger quotient = copy.numerator / copy.denominator;
        if (quotient != 0) {
            size_t old_size = (quotient).toString().size();
            for (size_t i = 0; i < 1 + (precision - 1) / 9; ++i) {
                copy.numerator *= base;
            }
            no_coma += (copy.numerator / copy.denominator).toString();
            if (copy.numerator < 0) {
                res += '-';
                no_coma = no_coma.substr(1);
            }
            res += no_coma.substr(0, old_size);
            res += '.';
            res += no_coma.substr(old_size, precision);
        } else {
            if (copy.numerator < 0)
                res += '-';
            res += (copy.numerator / copy.denominator).abs().toString();
            res += '.';
            BigInteger temp;
            temp = copy.numerator % copy.denominator;
            for (size_t i = 0; i < 1 + (precision - 1) / 9; ++i) {
                temp *= base;
                if ((temp / copy.denominator).abs().toString() == "0")
                    res += "000000000";
                else
                    res += (temp / copy.denominator).abs().toString();
                temp = temp % copy.denominator;
            }
            for (size_t i = 0; i < (1 + (precision - 1) / 9) % 9 - 1; ++i)
                res.pop_back();
        }
        return res;
    }

    explicit operator double();

};

Rational operator+(const Rational &first, const Rational &second) {
    Rational copy = first;
    copy += second;
    return copy;
}

Rational operator-(const Rational &first, const Rational &second) {
    Rational copy = first;
    copy -= second;
    return copy;
}

Rational operator*(const Rational &first, const Rational &second) {
    Rational copy = first;
    copy *= second;
    return copy;
}

Rational operator/(const Rational &first, const Rational &second) {
    Rational copy = first;
    copy /= second;
    return copy;
}

bool operator!=(const Rational &twin1, const Rational &twin2) {
    return !(twin1 == twin2);
}

bool operator<=(const Rational &twin1, const Rational &twin2) {
    return !(twin1 > twin2);
}

bool operator>=(const Rational &twin1, const Rational &twin2) {
    return !(twin1 < twin2);
}

Rational::operator double() {
    return std::stod(asDecimal(1000));
}
