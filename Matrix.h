#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>

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
    static const int base = 1e9;
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
        sign = (value >= 0);
        if (value < 0)
            value *= (-1);
        do {
            num.push_back(value % base);
            value /= base;
        } while (value > 0);
    }

    BigInteger(const std::string &str) {
        if (str == "-0" || str == "0") {
            sign = true;
            num.push_back(0);
            return;
        }
        size_t minus_shift = 0;
        if (str[0] == '-')
            minus_shift = 1;
        sign = !minus_shift;
        for (long long i = static_cast<long long>(str.size()) - 9; i >= 1; i -= 9) {
            num.push_back(std::stoi(str.substr(i, 9)));
        }
        if (str.size() % 9 != minus_shift)
            num.push_back(std::stoi(str.substr(minus_shift, (str.size() % 9 + 9 - minus_shift) % 9)));
    }

    explicit operator bool() const {
        return !(num.size() == 1 && 0 == num[0]);
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

    bool even() {
        return num[0] % 2 == 0;
    }

    BigInteger &sum(const BigInteger &number) {
        num.resize(std::max(num.size(), number.num.size()) + 1);
        for (size_t i = 0; i < number.num.size(); ++i) {
            if (num[i] + number.num[i] >= base) {
                num[i + 1] += (num[i] + number.num[i]) / base;
            }
            num[i] = (num[i] + number.num[i]) % base;
        }
        remove_zeros();
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
        if (!*this) {
            *this = -number;
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

        if (num.size() == 0) {
            sign = true;
            num.assign(1, 0);
        }

        for (size_t i = 0; i < res.num.size() - 1; ++i) {
            if (res.num.size() == 0) {
                res.sign = true;
                res.num.assign(1, 0);
            }
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
            quotient.num[i] = left;
        }
        if (changed)
            number.sign = false;
        num = quotient.num;
        remove_zeros();
        return *this;
    }

    BigInteger &operator%=(const BigInteger& number) {
        *this -= number * (*this / number);
        remove_zeros();
        return *this;
    }

    std::string toString() const {
        std::string str;
        str.reserve(9*num.size() + 1);
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

    friend bool operator==(const BigInteger &twin1, const BigInteger &twin2);

    friend bool operator>(const BigInteger &twin1, const BigInteger &twin2);

    friend bool operator<(const BigInteger &twin1, const BigInteger &twin2);

    BigInteger operator-() const {
        BigInteger res = *this;
        if (num.size() == 1 && num[0] == 0) { return res; }
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

    BigInteger &operator/=(long long number) {
        for (long long i = static_cast<long long>(num.size()) - 1; i >= 0; --i) {
            if (i > 0)
                num[i - 1] += (num[i] % number) * base;
            num[i] /= number;
        }
        remove_zeros();
        return *this;
    }

    BigInteger gcd(BigInteger b) const {
        BigInteger ans = 1, a = *this;
        if (a < 0)
            a *= -1;
        if (b < 0)
            b *= -1;
        while (a != 0 && b != 0) {
            if (!a.even() && !b.even()) {
                (a > b) ? (a -= b) : (b -= a);
            }
            if (a.even() && b.even()) {
                a /= 2;
                b /= 2;
                ans *= 2;
            }
            if (a.even() && !b.even()) {
                a /= 2;
            }
            if (!a.even() && b.even()) {
                b /= 2;
            }
        }

        ans *= (a == 0) ? b : a;
        return ans;
    }
};

bool operator==(const BigInteger &twin1, const BigInteger &twin2) {
    if (twin1.sign != twin2.sign)
        return false;
    return twin1.num == twin2.num;
}

bool operator>(const BigInteger &twin1, const BigInteger &twin2){
    if (twin1.sign && !twin2.sign)
        return true;
    if (!twin1.sign && twin2.sign)
        return false;
    if (twin1.sign) {
        if (twin1.num.size() != twin2.num.size()) return (twin1.num.size() > twin2.num.size());
        for (long long i = static_cast<long long>(twin1.num.size()) - 1; i >= 0; --i) {
            if (twin1.num[i] != twin2.num[i]) return (twin1.num[i] > twin2.num[i]);
        }
    }
    if (!twin1.sign) {
        if (twin1.num.size() != twin2.num.size()) return (twin1.num.size() < twin2.num.size());
        for (long long i = static_cast<long long>(twin1.num.size()) - 1; i >= 0; --i) {
            if (twin1.num[i] != twin2.num[i]) return (twin1.num[i] < twin2.num[i]);
        }
    }
    return false;
}

bool operator<(const BigInteger &twin1, const BigInteger &twin2) {
    return !(twin1 > twin2 || twin1 == twin2);
}

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
    static const int base = 1e9;

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

    friend bool operator==(const Rational &twin1, const Rational &twin2);

    friend bool operator>(const Rational &twin1, const Rational &twin2);

    friend std::istream &operator>>(std::istream &stream, Rational &num);

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
        no_coma.reserve(precision + copy.numerator.toString().size() + 1);
        res.reserve(precision + copy.numerator.toString().size() + 2);

        if (copy.numerator < 0)
            res += '-';
        res += (copy.numerator / copy.denominator).abs().toString();
        res += '.';
        BigInteger temp;
        temp = copy.numerator % copy.denominator;
        for (size_t i = 0; i < 9 - ((temp * base) / copy.denominator).abs().toString().size(); ++i) {
            res += "0";
        }
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

        return res;
    }

    explicit operator double() const;

    explicit operator int() const {
        return std::stoi(toString());
    }
};

bool operator==(const Rational &twin1, const Rational &twin2) {
    if (twin1.numerator != twin2.numerator)
        return false;
    if (twin1.denominator != twin2.denominator)
        return false;
    return true;
}

bool operator>(const Rational &twin1, const Rational &twin2) {
    return twin1.numerator * twin2.denominator > twin2.numerator * twin1.denominator;
}

bool operator<(const Rational &twin1, const Rational &twin2) {
    return !(twin1 > twin2 || twin1 == twin2);
}

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

std::istream &operator>>(std::istream &stream, Rational &number) {
    std::string str;
    stream >> str;
    number.numerator = str;
    number.denominator = 1;
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const Rational &number) {
    stream << number.toString();
    return stream;
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

Rational::operator double() const {
    return std::stod(asDecimal(30));
}





template <size_t N>
class Residue {
private:
    int number;

    template <size_t D, size_t P, bool greater_than_root = false>
    struct Divides {
        static const bool result = (P % D) && Divides<D + 1, P, (D*D > P)>::result;
    };

    template <size_t D, size_t P>
    struct Divides<D, P, true> {
        static const bool result = true;
    };

public:
    explicit Residue(long long num) {
        if (num >= 0)
            number = num % int(N);
        else {
            while (num < 0)
                num += int(N);
            number = num % int(N);
        }
    };

    Residue power_for_fermat(size_t ex) const {
        if (ex == 0) return Residue(1);
        if (ex % 2 == 1)
            return power_for_fermat(ex - 1) * (*this);
        else
            return power_for_fermat(ex / 2) * power_for_fermat(ex / 2);
    }

    void normalize() {
        if (number >= 0)
            number = number % int(N);
        else {
            while (number < 0)
                number += int(N);
            number = number % int(N);
        }
    }

    Residue &operator+=(const Residue &num) {
        number = (number + num.number) % int(N);
        normalize();
        return *this;
    }

    Residue &operator-=(const Residue &num) {
        number = (number - num.number) % int(N);
        normalize();
        return *this;
    }

    Residue &operator*=(const Residue &num) {
        number = (number * num.number) % int(N);
        normalize();
        return *this;
    }

    Residue &operator/=(const Residue &num) {
        static_assert(Divides<2, N, false>::result);
        *this *= num.power_for_fermat(N - 2);
        normalize();
        return *this;
    }

    Residue operator-() const {
        long long res = number;
        return Residue(-res);
    }

    std::istream &operator>>(std::istream &stream) {
        stream >> number;
        return stream;
    }

    std::ostream &operator<<(std::ostream &stream) {
        stream << number;
        return stream;
    }

    template<size_t S>
    friend bool operator==(const Residue<S> &twin1, const Residue<S> &twin2);

    explicit operator int() const {
        return number;
    }
};

template<size_t N>
Residue<N> operator+(const Residue<N> &first, const Residue<N> &second) {
    Residue copy = first;
    copy += second;
    return copy;
}
template<size_t N>
Residue<N> operator-(const Residue<N> &first, const Residue<N> &second) {
    Residue copy = first;
    copy -= second;
    return copy;
}

template<size_t N>
Residue<N> operator*(const Residue<N> &first, const Residue<N> &second) {
    Residue copy = first;
    copy *= second;
    return copy;
}

template<size_t N>
Residue<N> operator/(const Residue<N> &first, const Residue<N> &second) {
    Residue copy = first;
    copy /= second;
    return copy;
}

template<size_t N>
bool operator==(const Residue<N> &twin1, const Residue<N> &twin2) {
    return twin1.number == twin2.number;
}

template<size_t N>
bool operator!=(const Residue<N> &twin1, const Residue<N> &twin2) {
    return !(twin1 == twin2);
}

template <size_t N, size_t M, typename Field = Rational>
class Matrix {
private:
    std::vector<std::vector<Field>> cell;
public:
    Matrix() : cell(std::vector<std::vector<Field>> (N, std::vector<Field> (M, Field(0)))) {}
    ~Matrix() = default;

    Matrix(std::initializer_list<std::vector<int>> list) : cell(std::vector<std::vector<Field>> (N, std::vector<Field> (M, Field(0)))){
        size_t i = 0;
        size_t j = 0;
        for (const auto& x : list) {
            for (auto y: x) {
                cell[i][j] = Field(y);
                ++j;
            }
            j = 0;
            ++i;
        }
    }

    const std::vector<Field>& operator[](size_t ind) const {
        return cell[ind];
    }

    std::vector<Field>& operator[](size_t ind) {
        return cell[ind];
    }

    std::vector<Field> getRow(size_t ind) {
        return cell[ind];
    }

    std::vector<Field> getColumn(size_t ind) {
        std::vector<Field> column;
        for (size_t i = 0; i < N; ++i) {
            column.push_back(cell[i][ind]);
        }
        return column;
    }

    Matrix<N, M, Field> &scale(const Field& multiple) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                cell[i][j] *= multiple;
            }
        }
        return *this;
    }

    template<size_t A, size_t B>
    Matrix<N, M, Field> &operator+=(const Matrix<A, B, Field> &mat) {
        static_assert(N == A && M == B);
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                cell[i][j] += mat[i][j];
            }
        }
        return *this;
    }

    Matrix<N, M, Field> &operator*=(const Field& x) {
        scale(x);
        return *this;
    }

    template<size_t A, size_t B>
    Matrix<N, M, Field> &operator-=(const Matrix<A, B, Field> &mat) {
        static_assert(N == A && M == B);
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                cell[i][j] -= mat[i][j];
            }
        }
        return *this;
    }

    template<size_t A, size_t B>
    Matrix<N, M, Field>& operator*=(const Matrix<A, B, Field> &mat) {
        static_assert(M == A && A == B);
        Matrix<N, M, Field> copy;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                for (size_t r = 0; r < M; ++r) {
                    copy[i][j] += cell[i][r] * mat[r][j];
                }
            }
        }

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                cell[i][j] = copy[i][j];
            }
        }
        return *this;
    }

    template<size_t A, size_t B>
    Matrix<N, M, Field> operator+(const Matrix<A, B, Field> &mat) const{
        Matrix<N, M, Field> copy = *this;
        copy += mat;
        return copy;
    }

    template<size_t A, size_t B>
    Matrix<N, M, Field> operator-(const Matrix<A, B, Field> &mat) const{
        Matrix<N, M, Field> copy = *this;
        copy -= mat;
        return copy;
    }

    Matrix<M, N, Field> transposed() const {
        Matrix<M, N, Field> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                result[j][i] = cell[i][j];
            }
        }
        return result;
    }

    Field trace() {
        static_assert(N == M);
        Field sum = Field(0);
        for (size_t i = 0; i < N; ++i) {
            sum += cell[i][i];
        }
        return sum;
    }

    size_t rank() const {
        Matrix<N, M, Field> copy = *this;
        copy.gauss();
        size_t rank = 0;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                if (copy[i][j] != Field(0)) {
                    ++rank;
                    break;
                }
            }
        }
        return rank;
    }

    Matrix<N, M, Field>& invert() {
        static_assert(N == M);
        Matrix<N, 2 * M, Field> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                result[i][j] = cell[i][j];
            }
            result[i][M + i] = Field(1);
        }
        result.gauss();
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                cell[i][j] = result[i][j + M];
            }
        }
        return *this;
    }

    Matrix<M, N, Field> inverted() const {
        Matrix<N, M, Field> result = *this;
        result.invert();
        return result;
    }

    std::pair<Matrix<N, M, Field>&, Field> gauss() {
        bool even = true;
        size_t j = 0;
        Field ratio(Field(1));
        Field factor(Field(1));
        for (size_t i = 0; i < std::min(N, M); ++i) {
            j = i;
            even = true;
            while (j < N && cell[j][i] == Field(0)) {
                ++j;
                even = !even;
            }
            if (j == N)
                continue;
            if (!even) {
                ratio = -ratio;
                std::swap(cell[i], cell[j]);
            }
            j = 0;
            while (j < N) {
                if (i == j) {
                    ++j;
                    continue;
                }
                factor = cell[j][i]/cell[i][i];
                if (factor == Field(0)) {
                    ++j;
                    continue;
                }
                for (size_t r = i; r < M; ++r) {
                    cell[j][r] -= factor * cell[i][r];
                }
                ++j;
            }

            factor = Field(1)/cell[i][i];
            ratio *= cell[i][i];
            for (size_t r = i; r < M; ++r) {
                cell[i][r] *= factor;
            }
        }

        return {*this, ratio};
    }

    Field det() {
        static_assert(N == M);
        Matrix copy = *this;
        return copy.gauss().second;
    }
};

template<size_t A, size_t B, size_t C, size_t D, typename Field>
Matrix<A, D, Field> operator*(const Matrix<A, B, Field> &mat1, const Matrix<C, D, Field> &mat2) {
    static_assert(B == C);
    Matrix<A, D, Field> copy;
    for (size_t i = 0; i < A; ++i) {
        for (size_t j = 0; j < D; ++j) {
            for (size_t r = 0; r < B; ++r) {
                copy[i][j] += mat1[i][r] * mat2[r][j];
            }
        }
    }
    return copy;
}

template<size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator*(const Field& x, const Matrix<N, M, Field>& mat) {
    Matrix<N, M, Field> copy = mat;
    copy *= x;
    return copy;
}

template<size_t A, size_t B, size_t C, size_t D, typename Field = Rational>
bool operator==(const Matrix<A, B, Field> &twin1, const Matrix<C, D, Field> &twin2) {
    if (A != C || B != D)
        return false;
    for (size_t i = 0; i < A; ++i) {
        for (size_t j = 0; j < B; ++j) {
            if (twin1[i][j] != twin2[i][j])
                return false;
        }
    }
    return true;
}

template<size_t A, size_t B, size_t C, size_t D, typename Field = Rational>
bool operator!=(const Matrix<A, B, Field> &twin1, const Matrix<C, D, Field> &twin2) {
    return !(twin1 == twin2);
}

template<size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;
