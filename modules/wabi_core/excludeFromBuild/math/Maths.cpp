
template<> const float Mathf::EPSILON = FLT_EPSILON;
template<> const float Mathf::ZERO_TOLERANCE = 1e-06f;
template<> const float Mathf::MAX_REAL = FLT_MAX;
template<> const float Mathf::PI = (float)(4.0*atan(1.0));
template<> const float Mathf::TWO_PI = 2.0f*Mathf::PI;
template<> const float Mathf::HALF_PI = 0.5f*Mathf::PI;
template<> const float Mathf::INV_PI = 1.0f/Mathf::PI;
template<> const float Mathf::INV_TWO_PI = 1.0f/Mathf::TWO_PI;
template<> const float Mathf::DEG_TO_RAD = Mathf::PI/180.0f;
template<> const float Mathf::RAD_TO_DEG = 180.0f/Mathf::PI;
template<> const float Mathf::LN_2 = Mathf::Log(2.0f);
template<> const float Mathf::LN_10 = Mathf::Log(10.0f);
template<> const float Mathf::INV_LN_2 = 1.0f/Mathf::LN_2;
template<> const float Mathf::INV_LN_10 = 1.0f/Mathf::LN_10;
template<> const float Mathf::SQRT_2 = (float)(sqrt(2.0));
template<> const float Mathf::INV_SQRT_2 = 1.0f/Mathf::SQRT_2;
template<> const float Mathf::SQRT_3 = (float)(sqrt(3.0));
template<> const float Mathf::INV_SQRT_3 = 1.0f/Mathf::SQRT_3;

template<> const double Math<double>::EPSILON = DBL_EPSILON;
template<> const double Math<double>::ZERO_TOLERANCE = 1e-08;
template<> const double Math<double>::MAX_REAL = DBL_MAX;
template<> const double Math<double>::PI = 4.0*atan(1.0);
template<> const double Math<double>::TWO_PI = 2.0*Math<double>::PI;
template<> const double Math<double>::HALF_PI = 0.5*Math<double>::PI;
template<> const double Math<double>::INV_PI = 1.0/Math<double>::PI;
template<> const double Math<double>::INV_TWO_PI = 1.0/Math<double>::TWO_PI;
template<> const double Math<double>::DEG_TO_RAD = Math<double>::PI/180.0;
template<> const double Math<double>::RAD_TO_DEG = 180.0/Math<double>::PI;
template<> const double Math<double>::LN_2 = Math<double>::Log(2.0);
template<> const double Math<double>::LN_10 = Math<double>::Log(10.0);
template<> const double Math<double>::INV_LN_2 = 1.0/Math<double>::LN_2;
template<> const double Math<double>::INV_LN_10 = 1.0/Math<double>::LN_10;
template<> const double Math<double>::SQRT_2 = sqrt(2.0);
template<> const double Math<double>::INV_SQRT_2 = 1.0f/Mathf::SQRT_2;
template<> const double Math<double>::SQRT_3 = sqrt(3.0);
template<> const double Math<double>::INV_SQRT_3 = 1.0f/Mathf::SQRT_3;

template <typename T>
T Math<T>::ACos (T value)
{
    if (-(T)1 < value)
    {
        if (value < (T)1)
        {
            return acos(value);
        }
        else
        {
            return (T)0;
        }
    }
    else
    {
        return PI;
    }
}

template <typename T>
T Math<T>::ASin (T value)
{
    if (-(T)1 < value)
    {
        if (value < (T)1)
        {
            return asin(value);
        }
        else
        {
            return HALF_PI;
        }
    }
    else
    {
        return -HALF_PI;
    }
}

template <typename T>
T Math<T>::ATan (T value)
{
    return atan(value);
}

template <typename T>
T Math<T>::ATan2 (T y, T x)
{
    if (x != (T)0 || y != (T)0)
    {
        return atan2(y, x);
    }
    else
    {
        // Mathematically, ATan2(0,0) is undefined, but ANSI standards
        // require the function to return 0.
        return (T)0;
    }
}

template <typename T>
T Math<T>::Ceil (T value)
{
    return ceil(value);
}

template <typename T>
T Math<T>::Cos (T value)
{
    return cos(value);
}

template <typename T>
T Math<T>::Exp (T value)
{
    return exp(value);
}

template <typename T>
T Math<T>::FAbs (T value)
{
    return fabs(value);
}

template <typename T>
T Math<T>::Floor (T value)
{
    return floor(value);
}

template <typename T>
T Math<T>::FMod (T x, T y)
{
    if (y != (T)0)
    {
        return fmod(x, y);
    }
    else
    {
		// Zero input to FMod
        assert(false);
        return (T)0;
    }
}

template <typename T>
T Math<T>::InvSqrt (T value)
{
    if (value != (T)0)
    {
        return ((T)1)/sqrt(value);
    }
    else
    {
		// Division by zero in InvSqr
        assert(false);
        return (T)0;
    }
}

template <typename T>
T Math<T>::Log (T value)
{
    if (value > (T)0)
    {
        return log(value);
    }
    else
    {
		// Nonpositive input to Log
        assert(false); 
        return (T)0;
    }
}

template <typename T>
T Math<T>::Log2 (T value)
{
    if (value > (T)0)
    {
        return Math<T>::INV_LN_2 * log(value);
    }
    else
    {
		// Nonpositive input to Log2
        assert(false);
        return (T)0;
    }
}

template <typename T>
T Math<T>::Log10 (T value)
{
    if (value > (T)0)
    {
        return Math<T>::INV_LN_10 * log(value);
    }
    else
    {
		// Nonpositive input to Log10
        assert(false);
        return (T)0;
    }
}

template <typename T>
T Math<T>::Pow (T base, T exponent)
{
    if (base >= (T)0)
    {
        return pow(base, exponent);
    }
    else
    {
		// Negative base not allowed in Pow
        assert(false);
        return Math<T>::MAX_REAL;
    }
}

template <typename T>
T Math<T>::Sin (T value)
{
    return sin(value);
}

template <typename T>
T Math<T>::Sqr (T value)
{
    return value*value;
}

template <typename T>
T Math<T>::Sqrt (T value)
{
    if (value >= (T)0)
    {
        return sqrt(value);
    }
    else
    {
		// Negative input to Sqrt
        assert(false);
        return (T)0;
    }
}

template <typename T>
T Math<T>::Tan (T value)
{
    return tan(value);
}

template <typename T>
int Math<T>::sign (int value)
{
    if (value > 0)
    {
        return 1;
    }

    if (value < 0)
    {
        return -1;
    }

    return 0;
}

template <typename T>
T Math<T>::sign (T value)
{
    if (value > (T)0)
    {
        return (T)1;
    }

    if (value < (T)0)
    {
        return (T)-1;
    }

    return (T)0;
}

template <typename T>
T Math<T>::unitRandom (unsigned int seed)
{
    if (seed > 0)
    {
        srand(seed);
    }

    T ratio = ((T)rand())/((T)(RAND_MAX));
    return (T)ratio;
}

template <typename T>
T Math<T>::symetricRandom (unsigned int seed)
{
    return ((T)2)*unitRandom(seed) - (T)1;
}

template <typename T>
T Math<T>::intervalRandom (T min, T max, unsigned int seed)
{
    return min + (max - min)*unitRandom(seed);
}

template <typename T>
T Math<T>::clamp (T value, T minValue, T maxValue)
{
    if (value <= minValue)
    {
         return minValue;
    }
    if (value >= maxValue)
    {
        return maxValue;
    }
    return value;
}

template <typename T>
T Math<T>::saturate (T value)
{
    if (value <= (T)0)
    {
         return (T)0;
    }
    if (value >= (T)1)
    {
        return (T)1;
    }
    return value;
}

template <typename T>
T Math<T>::fastSin0 (T angle)
{
    T angleSqr = angle*angle;
    T result = (T)7.61e-03;
    result *= angleSqr;
    result -= (T)1.6605e-01;
    result *= angleSqr;
    result += (T)1.0;
    result *= angle;
    return result;
}

template <typename T>
T Math<T>::fastSin1 (T angle)
{
    T angleSqr = angle*angle;
    T result = -(T)2.39e-08;
    result *= angleSqr;
    result += (T)2.7526e-06;
    result *= angleSqr;
    result -= (T)1.98409e-04;
    result *= angleSqr;
    result += (T)8.3333315e-03;
    result *= angleSqr;
    result -= (T)1.666666664e-01;
    result *= angleSqr;
    result += (T)1.0;
    result *= angle;
    return result;
}

template <typename T>
T Math<T>::fastCos0 (T angle)
{
    T angleSqr = angle*angle;
    T result = (T)3.705e-02;
    result *= angleSqr;
    result -= (T)4.967e-01;
    result *= angleSqr;
    result += (T)1.0;
    return result;
}

template <typename T>
T Math<T>::fastCos1 (T angle)
{
    T angleSqr = angle*angle;
    T result = -(T)2.605e-07;
    result *= angleSqr;
    result += (T)2.47609e-05;
    result *= angleSqr;
    result -= (T)1.3888397e-03;
    result *= angleSqr;
    result += (T)4.16666418e-02;
    result *= angleSqr;
    result -= (T)4.999999963e-01;
    result *= angleSqr;
    result += (T)1.0;
    return result;
}

template <typename T>
T Math<T>::fastTan0 (T angle)
{
    T angleSqr = angle*angle;
    T result = (T)2.033e-01;
    result *= angleSqr;
    result += (T)3.1755e-01;
    result *= angleSqr;
    result += (T)1.0;
    result *= angle;
    return result;
}

template <typename T>
T Math<T>::fastTan1 (T angle)
{
    T angleSqr = angle*angle;
    T result = (T)9.5168091e-03;
    result *= angleSqr;
    result += (T)2.900525e-03;
    result *= angleSqr;
    result += (T)2.45650893e-02;
    result *= angleSqr;
    result += (T)5.33740603e-02;
    result *= angleSqr;
    result += (T)1.333923995e-01;
    result *= angleSqr;
    result += (T)3.333314036e-01;
    result *= angleSqr;
    result += (T)1.0;
    result *= angle;
    return result;
}

template <typename T>
T Math<T>::fastInvSin0 (T value)
{
    T root = Math<T>::Sqrt(FAbs((T)1 - value));
    T result = -(T)0.0187293;
    result *= value;
    result += (T)0.0742610;
    result *= value;
    result -= (T)0.2121144;
    result *= value;
    result += (T)1.5707288;
    result = HALF_PI - root*result;
    return result;
}

template <typename T>
T Math<T>::fastInvSin1 (T value)
{
    T root = Math<T>::Sqrt(FAbs((T)1 - value));
    T result = -(T)0.0012624911;
    result *= value;
    result += (T)0.0066700901;
    result *= value;
    result -= (T)0.0170881256;
    result *= value;
    result += (T)0.0308918810;
    result *= value;
    result -= (T)0.0501743046;
    result *= value;
    result += (T)0.0889789874;
    result *= value;
    result -= (T)0.2145988016;
    result *= value;
    result += (T)1.5707963050;
    result = HALF_PI - root*result;
    return result;
}

template <typename T>
T Math<T>::fastInvCos0 (T value)
{
    T root = Math<T>::Sqrt(FAbs((T)1 - value));
    T result = -(T)0.0187293;
    result *= value;
    result += (T)0.0742610;
    result *= value;
    result -= (T)0.2121144;
    result *= value;
    result += (T)1.5707288;
    result *= root;
    return result;
}

template <typename T>
T Math<T>::fastInvCos1 (T value)
{
    T root = Math<T>::Sqrt(FAbs((T)1 - value));
    T result = -(T)0.0012624911;
    result *= value;
    result += (T)0.0066700901;
    result *= value;
    result -= (T)0.0170881256;
    result *= value;
    result += (T)0.0308918810;
    result *= value;
    result -= (T)0.0501743046;
    result *= value;
    result += (T)0.0889789874;
    result *= value;
    result -= (T)0.2145988016;
    result *= value;
    result += (T)1.5707963050;
    result *= root;
    return result;
}

template <typename T>
T Math<T>::fastInvTan0 (T value)
{
    T valueSqr = value*value;
    T result = (T)0.0208351;
    result *= valueSqr;
    result -= (T)0.085133;
    result *= valueSqr;
    result += (T)0.180141;
    result *= valueSqr;
    result -= (T)0.3302995;
    result *= valueSqr;
    result += (T)0.999866;
    result *= value;
    return result;
}

template <typename T>
T Math<T>::fastInvTan1 (T value)
{
    T valueSqr = value*value;
    T result = (T)0.0028662257;
    result *= valueSqr;
    result -= (T)0.0161657367;
    result *= valueSqr;
    result += (T)0.0429096138;
    result *= valueSqr;
    result -= (T)0.0752896400;
    result *= valueSqr;
    result += (T)0.1065626393;
    result *= valueSqr;
    result -= (T)0.1420889944;
    result *= valueSqr;
    result += (T)0.1999355085;
    result *= valueSqr;
    result -= (T)0.3333314528;
    result *= valueSqr;
    result += (T)1;
    result *= value;
    return result;
}

template <typename T>
T Math<T>::fastNegExp0 (T value)
{
    T result = (T)0.0038278;
    result *= value;
    result += (T)0.0292732;
    result *= value;
    result += (T)0.2507213;
    result *= value;
    result += (T)1;
    result *= result;
    result *= result;
    result = ((T)1)/result;
    return result;
}

template <typename T>
T Math<T>::fastNegExp1 (T value)
{
    T result = (T)0.00026695;
    result *= value;
    result += (T)0.00227723;
    result *= value;
    result += (T)0.03158565;
    result *= value;
    result += (T)0.24991035;
    result *= value;
    result += (T)1;
    result *= result;
    result *= result;
    result = ((T)1)/result;
    return result;
}

template <typename T>
T Math<T>::fastNegExp2 (T value)
{
    T result = (T)0.000014876;
    result *= value;
    result += (T)0.000127992;
    result *= value;
    result += (T)0.002673255;
    result *= value;
    result += (T)0.031198056;
    result *= value;
    result += (T)0.250010936;
    result *= value;
    result += (T)1;
    result *= result;
    result *= result;
    result = ((T)1)/result;
    return result;
}

template <typename T>
T Math<T>::fastNegExp3 (T value)
{
    T result = (T)0.0000006906;
    result *= value;
    result += (T)0.0000054302;
    result *= value;
    result += (T)0.0001715620;
    result *= value;
    result += (T)0.0025913712;
    result *= value;
    result += (T)0.0312575832;
    result *= value;
    result += (T)0.2499986842;
    result *= value;
    result += (T)1;
    result *= result;
    result *= result;
    result = ((T)1)/result;
    return result;
}

template class Math<float>;
template class Math<double>;



