#ifndef _RMATH3D_H
#define _RMATH3D_H

#include <math.h>

#define PI (3.14159265358979323846)
#define PI_DIV_180 (0.017453292519943296)
#define _180_DIV_PI (57.2957795130823229)

namespace random
{


template<typename T>
    struct only_t
{
    typedef T type;
};

template<typename T>
struct only_t<T*>
{
    typedef typename only_t<T>::type type;
};

template<typename T>
struct only_t<const T>
{
    typedef typename only_t<T>::type type;
};

template<typename T>
struct only_t<T&>
{
    typedef T type;
};


template<typename T>
struct is_value_help
{
    const static bool value = false;
};

template<>
struct is_value_help<char>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef  short upgrade_type;
};
template<>
struct is_value_help<unsigned char>
{
    const static bool value = true;
    const static int unsigned_flag = 1;
    typedef  short upgrade_type;
};
template<>
struct is_value_help<short>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef  int upgrade_type;
};
template<>
struct is_value_help<unsigned short>
{
    const static bool value = true;
    const static int unsigned_flag = 1;
    typedef  int upgrade_type;
};
template<>
struct is_value_help<int>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef long long int upgrade_type;
};
template<>
struct is_value_help<unsigned int>
{
    const static bool value = true;
    const static int unsigned_flag = 1;
    typedef long long int upgrade_type;
};
template<>
struct is_value_help<long>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef long long int upgrade_type;
};
template<>
struct is_value_help<unsigned long>
{
    const static bool value = true;
    const static int unsigned_flag = 1;
    typedef long long int upgrade_type;
};
template<>
struct is_value_help<long long int>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef  long long int upgrade_type;
};

template<>
struct is_value_help<unsigned long long int>
{
    const static bool value = true;
    const static int unsigned_flag = 1;
    typedef  long long int upgrade_type;
};

template<>
struct is_value_help<float>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef float upgrade_type;
};

template<>
struct is_value_help<double>
{
    const static bool value = true;
    const static int unsigned_flag = -1;
    typedef  double upgrade_type;
};

template <typename T>
struct is_value
{
    const static bool value = is_value_help<typename only_t<T>::type>::value;
};


template<bool ISVALUE, typename T1, typename T2>
struct if_else;

template<typename T1, typename T2>
struct if_else<true, T1, T2>
{
    typedef T1 type;
};

template<typename T1, typename T2>
struct if_else<false, T1, T2>
{
    typedef T2 type;
};

template<bool ISOVER, typename T1, typename T2>
struct select_type_help;

template<typename T1, typename T2>
struct select_type_help<false, T1, T2>
{
    typedef typename select_type_help<true, T2, T1>::type type;
};

template<typename T1, typename T2>
struct select_type_help<true, T1, T2>
{
    const static bool value =  ((is_value_help<T1>::unsigned_flag * is_value_help<T2>::unsigned_flag) < 0)
     && ((is_value_help<T1>::unsigned_flag == 1) || (sizeof(T1) == sizeof(T2)));

    typedef typename if_else< value , typename is_value_help<T1>::upgrade_type, T1>::type type;
};


template<typename T>
struct select_type_help<true, T, T>
{
    typedef T type;
};

template<typename T>
struct select_type_help<true, double, T>
{
    typedef double type;
};

template<typename T>
struct select_type_help<true, T, double>
{
    typedef double type;
};

template<typename T>
struct select_type_help<false, float, T>
{   
    typedef typename if_else<sizeof(float) == sizeof(T), float, double>::type type;
};

template<typename T>
struct select_type_help<false, T, float>
{
    typedef float type;
};

template<typename T1, typename T2>
struct select_type
{
    typedef typename only_t<T1>::type OnlyT1;
    typedef typename only_t<T2>::type OnlyT2;
    typedef typename select_type_help<(sizeof(OnlyT1) > sizeof(OnlyT2)), OnlyT1, OnlyT2>::type type;
};

template<typename T>
struct select_type<T, T>
{
    typedef T type;
};


template<typename TYPE, int SIZE>
class vec
{
public:

    template<int NSize>
    void vec_help(){}
    
    template<int NSize, typename T0, typename ... TN>
    void vec_help(T0 t0, TN ... tn)
    {
        if (SIZE >= NSize)
        {
            v[SIZE - NSize] = (TYPE)t0;
        }
        vec_help<NSize - 1, TN...>(tn ...);
    }

    template<typename OT>
    vec(const vec < OT, SIZE>& other)
    {
        for (int i = 0; i < SIZE; ++i)
        {
            v[i] = (TYPE)other[i];
        }
    }

    template<typename OT>
    vec(const vec < OT, SIZE>&& other)
    {
        for (int i = 0; i < SIZE; ++i)
        {
            v[i] = (TYPE)other[i];
        }
    }

    template<typename ... TN>
    vec(TN ... tn)
    {
        static_assert(sizeof...(TN) == SIZE, "arg cnt error");
        vec_help<SIZE, TN ...>(tn ...);
    }

    vec()
    {
        for (int i = 0; i < SIZE; ++i)
        {
            v[i] = 0;
        }
    }

    ~vec() {}

    template<typename VT>
    typename select_type<TYPE, VT>::type operator *(const vec<VT, SIZE>& other) const
    {
        typename select_type<TYPE, VT>::type c = 0;
        for (int i = 0; i < SIZE; ++i)
        {
            c += (typename select_type<TYPE, VT>::type)v[i] * other[i];
        }
        return c;
    }

    template<typename VT>
    vec<typename select_type<TYPE, VT>::type, SIZE> x_help(VT value) const
    {
        vec<typename select_type<TYPE, VT>::type, SIZE> ret;
        for (int i = 0; i < SIZE; ++i)
        {
            ret[i] = (typename select_type<TYPE, VT>::type)(v[i] * value);
        }
        return ret;
    }

    vec<typename select_type<TYPE, int>::type, SIZE> operator *(int value) const
    {
        return x_help<int>(value);
    }
    vec<typename select_type<TYPE, unsigned int>::type, SIZE> operator *(unsigned int value) const
    {
        return x_help<unsigned int>(value);
    }
    vec<typename select_type<TYPE, float>::type, SIZE> operator *(float value) const
    {
        return x_help<float>(value);
    }

    vec<typename select_type<TYPE, double>::type, SIZE> operator *(double value) const
    {
        return x_help<double>(value);;
    }

    template<typename VT>
    vec<typename select_type<VT, TYPE>::type, SIZE> operator +(const vec<VT, SIZE>& other) const
    {
        vec<typename select_type<VT, TYPE>::type, SIZE> res;

        for(int i = 0; i < SIZE; ++i)
        {
            res[i] = v[i] + other[i];
        }
        return res;
    }

    template<typename VT>
    vec<typename select_type<TYPE, VT>::type, SIZE> operator -(const vec<VT, SIZE>& other) const
    {
        vec<typename select_type<TYPE, VT>::type, SIZE> res;

        for (int i = 0; i < SIZE; ++i)
        {
            res[i] = v[i] - other[i];
        }
        return res;
    }
    
    template<typename ST>
    vec<typename select_type<TYPE, ST>::type, SIZE> operator /(ST scale) const
    {
        vec<typename select_type<TYPE, ST>::type, SIZE> res;

       for(int i = 0; i < SIZE; ++i)
        {
            res[i] = v[i] / scale;
        }
        return res;
    }


    template<typename VT>
    void operator /=(VT value)
    {
        for (int i = 0; i < SIZE; ++i)
        {
             v[i] = (TYPE)(v[i] / value);
        }
    }

    template<typename VT>
    void operator *=(VT value)
    {
        for (int i = 0; i < SIZE; ++i)
        {
             v[i] = (TYPE)(v[i] * value);
        }
    }

    template<typename VT>
    void operator +=(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            v[i] = (TYPE)(v[i] + other[i]);
        }
    }

    template<typename VT>
    void operator -=(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            v[i] = (TYPE)(v[i] - other[i]);
        }
    }

    template<typename VT>
    bool operator ==(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] != other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename VT>
    bool operator !=(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] == other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename VT>
    bool operator >(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] <= other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename VT>
    bool operator >=(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] < other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename VT>
    bool operator <(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] >= other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<typename VT>
    bool operator <=(const vec<VT, SIZE>& other)
    {
        for(int i = 0; i < SIZE; ++i)
        {
            if(v[i] > other[i])
            {
                return false;
            }
        }
        return true;
    }

    template<int NewSize>
    vec<TYPE, NewSize> cut()
    {
        static_assert(NewSize <= SIZE, "cut size too long");
        vec<TYPE, NewSize> res;
        for(int i = 0; i < NewSize; ++i)
        {
            res[i] = v[i];
        }
        return res;
    }

    size_t size() const {return SIZE;}

    double length()
    {
        double c = 0;
        for (int i = 0; i < SIZE; ++i)
        {
            c += (double)v[i] * v[i];
        }
        return sqrt(c);
    }

    vec<TYPE, SIZE>& normalize()
    {
        operator*=<double>(1.0f / length());
        return *this;
    }

    // TYPE operator[](unsigned int index) const
    // {
    //     if (index >= SIZE) return 0;
    //     return v[index];
    // }

    const TYPE& operator[](unsigned int index) const
    {
        return v[index];
    }

    TYPE& operator[](unsigned int index)
    {
        return v[index];
    }


private:
    TYPE v[SIZE];
};

template<typename TYPE1, typename TYPE2>
vec<typename select_type<TYPE1, TYPE2>::type, 3>
cross_product(const vec<TYPE1, 3>& u, const vec<TYPE2, 3>& v)
{
    vec<typename select_type<TYPE1, TYPE2>::type, 3> res;
    res[0] = u[1]*v[2] - v[1]*u[2];
    res[1] = -u[0]*v[2] + v[0]*u[2];
    res[2] = u[0]*v[1] - v[0]*u[1];
    return res;
}


template<typename T1, typename T2>
vec<typename select_type<T1, T2>::type, 3> normal_vector(const vec<T1, 3>& t1, const vec<T2, 3>& t2)
{
    //a x b = (a2b3 - a3b2, -(a1b3 - a3b1), a1b2 - a2b1) .
    vec<typename select_type<T1, T2>::type, 3> res;
    for (int i = 0; i < 3; ++i)
    {
        int i1 = (i + 1) % 3;
        int i2 = (i + 2) % 3;
        res[i] =  t1[i1] * t2[i2] - t1[i2] * t2[i1];
    }
    return res;
}

/**
 * fit to opengl/glsl mat
 * v00 v10 v20 v30
 * v01 v11 v21 v31
 * v02 v12 v22 v32
 * v03 v13 v23 v33
 * 
 */
template<typename TYPE, int YSIZE, int XSIZE>
class mat
{
private:
    vec<TYPE, XSIZE>       v[YSIZE];

    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE>
    x_help(VT value) const
    {
        mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> ret;
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                ret[i][j] = (typename select_type<TYPE, VT>::type)(v[i][j] * value);
            }
        }
        return ret;
    }
public:

    template<typename OT>
    mat(const mat<OT, YSIZE, XSIZE>&& other)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)other[i][j];
            }
        }
    }

    template<typename OT>
    mat(const mat<OT, YSIZE, XSIZE>& other)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)other[i][j];
            }
        }
    }


    template<int NSize>
    void mat_help(){}
    
    template<int NSize, typename T0, typename ... TN>
    void mat_help(T0 t0, TN ... tn)
    {
        int index = 0;
        if (XSIZE*YSIZE >= NSize)
        {
            index = XSIZE*YSIZE - NSize;
            v[index/XSIZE][index%XSIZE] = (TYPE)t0;
        }
        mat_help<NSize - 1, TN...>(tn ...);
    }

    template<typename ... TN>
    mat(TN ... tn)
    {
        static_assert(sizeof...(TN) == XSIZE * YSIZE, "arg cnt error");
        mat_help<XSIZE * YSIZE, TN ...>(tn ...);
    }

    mat(TYPE t)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (i == j) ? t : 0;
            }
        }
    }

    mat()
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = 0;
            }
        }
    }

    ~mat() {}


    size_t size() const {return YSIZE;}


    mat<typename select_type<TYPE, unsigned int>::type, YSIZE, XSIZE> 
    operator *(unsigned int value) const
    {
        return x_help<unsigned int>(value);
    }

    mat<typename select_type<TYPE, int>::type, YSIZE, XSIZE> 
    operator *(int value) const
    {
        return x_help<int>(value);
    }

    mat<typename select_type<TYPE, float>::type, YSIZE, XSIZE> 
    operator *(float value) const
    {
        return x_help<float>(value);
    }
 
    mat<typename select_type<TYPE, double>::type, YSIZE, XSIZE> 
    operator *(double value) const
    {
        return x_help<double>(value);
    }

    template<typename VT>
    vec<typename select_type<TYPE, VT>::type, XSIZE> 
    operator *(const vec<VT, YSIZE>& other) const
    {
        vec<typename select_type<TYPE, VT>::type, XSIZE> res;
        typename select_type<TYPE, VT>::type c;
        for(int i = 0; i < XSIZE; ++i)
        {
            c = 0;
            for(int j = 0; j < YSIZE; ++j)
            {
                c += v[j][i] * other[j];
            }
            res[i] = c;
        }
        return res;
    }

    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, XSIZE, XSIZE> 
    operator *(const mat<VT, XSIZE, YSIZE>& other) const
    {
        // fit to opengl mat
        mat<typename select_type<TYPE, VT>::type, YSIZE, YSIZE> res;
        typename select_type<TYPE, VT>::type c;
        for (int i = 0; i < XSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                c = 0;
                for(int r = 0; r < YSIZE; ++r)
                {           
                    c += v[r][j] * other[i][r];
                }
                res[i][j] = c;
            }
        }
        return res;
    }


    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> 
    operator /(VT value) const
    {
        mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> ret;
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                ret[i][j] = (typename select_type<TYPE, VT>::type)(v[i][j] / value);
            }
        }
        return ret;
    }

    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> 
    operator +(const mat<VT, YSIZE, XSIZE>& other) const
    {
        mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> res;
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                res[i][j] = (typename select_type<TYPE, VT>::type)(v[i][j] + other[i][j]);
            }
        }
        return res;
    }

    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> 
    operator -(const mat<VT, YSIZE, XSIZE>& other) const
    {
        mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> res;
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                res[i][j] = (typename select_type<TYPE, VT>::type)(v[i][j] + other[i][j]);
            }
        }
        return res;
    }

    //Hadamard x
    template<typename VT>
    mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> 
    hadamard(const mat<VT, YSIZE, XSIZE>& other) const
    {
        mat<typename select_type<TYPE, VT>::type, YSIZE, XSIZE> res;
        typename select_type<TYPE, VT>::type c;
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                res[i][j] = v[i][j] * other[i][j];
            }
        }
        return res;
    }

    template<typename VT>
    void operator /=(VT value)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)(v[i][j] / value);
            }
        }
    }

    template<typename VT>
    void operator *=(VT value)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)(v[i][j] * value);
            }
        }
    }

    template<typename VT>
    void operator +=(const mat<VT, YSIZE, XSIZE>& other)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)(v[i][j] + other[i][j]);
            }
        }
    }

    template<typename VT>
    void operator -=(const mat<VT, YSIZE, XSIZE>& other)
    {
        for (int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                v[i][j] = (TYPE)(v[i][j] - other[i][j]);
            }
        }
    }

    template<typename VT>
    bool operator ==(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] != other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename VT>
    bool operator !=(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] == other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename VT>
    bool operator >=(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] < other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename VT>
    bool operator >(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] <= other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename VT>
    bool operator <=(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] > other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename VT>
    bool operator <(const mat<VT, YSIZE, XSIZE>& other)
    {
        for(int i = 0; i < YSIZE; ++i)
        {
            for(int j = 0; j < XSIZE; ++j)
            {
                if(v[i][j] >= other[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }


    const vec<TYPE, XSIZE>&  operator[](unsigned int index) const
    {
        return v[index];
    }

    vec<TYPE, XSIZE>&  operator[](unsigned int index)
    {
        return v[index];
    }

    mat<TYPE, XSIZE, YSIZE> operator !() const
    {
        mat<TYPE, XSIZE, YSIZE> res;
        for(int i = 0; i < XSIZE; ++i)
        {
            for(int j = 0; j < YSIZE; ++j)
            {
                res[i][j] = v[j][i];
            }
        }
        return res;
    }


};

template<typename T>
struct is_float
{
    typedef long long int type;
    static const bool value = false;
};

template<>
struct is_float<float>
{
    static const bool value = true;
    typedef double type;
};

template<>
struct is_float<double>
{
    static const bool value = true;
    typedef double type;
};



template<typename TYPE, int SIZE>
typename is_float<TYPE>::type det(const mat<TYPE, SIZE, SIZE>& m)
{
    typedef typename is_float<TYPE>::type ResType;
    ResType s = 0;
    mat<TYPE, SIZE - 1, SIZE - 1> mm;
    for(int t = 0; t < SIZE; ++t)
    {
        for(int i = 1; i < SIZE; ++i)
        {
            for(int j = 0; j < t; ++j)
            {
                mm[i-1][j] = m[i][j];
            }

            for(int j = t + 1; j < SIZE; ++j)
            {
                mm[i-1][j - 1] = m[i][j];
            }
        }
        s += (ResType)m[0][t] * ((t % 2) * (-2) + 1) * det(mm);
    }
    return s;
}

template<typename TYPE>
typename is_float<TYPE>::type det(const mat<TYPE, 3, 3>& m)
{
    typedef typename is_float<TYPE>::type ResType;
    return (ResType)m[0][0] * m[1][1] * m[2][2] + (ResType)m[0][1]*m[1][2]*m[2][0] + (ResType)m[0][2]*m[1][0]*m[2][1] - (ResType)m[0][0] * m[1][2] * m[2][1] - (ResType)m[0][1]*m[1][0]*m[2][2] - (ResType)m[0][2] * m[1][1]*m[2][0];
}

template<typename TYPE>
typename is_float<TYPE>::type det(const mat<TYPE, 2, 2>& m)
{
    typedef typename is_float<TYPE>::type ResType;
    return (ResType)m[0][0] * m[1][1] - (ResType)m[0][1] * m[1][0];
}

template<typename TYPE>
typename is_float<TYPE>::type det(const mat<TYPE, 1, 1>& m)
{
 typedef typename is_float<TYPE>::type ResType;
 return m[0][0];
}

template<typename TYPE, int SIZE>
mat<TYPE, SIZE, SIZE> adjoint(const mat<TYPE, SIZE, SIZE>& m)
{
    int x1, y1, x2, y2;
    mat<TYPE, SIZE, SIZE> res;
    mat<TYPE, SIZE - 1, SIZE - 1> mm;

    for(int i = 0; i < SIZE; ++i)
    {
        for(int j = 0; j < SIZE; ++j)
        {
            for(int t1 = 0; t1 < i; ++t1)
            {
                for(int t2 = 0; t2 < j; ++t2)
                {
                    mm[t1][t2] = m[t1][t2];
                }
                for(int t2 = j + 1; t2 < SIZE; ++t2)
                {
                    mm[t1][t2 - 1] = m[t1][t2];
                }
            }
            for(int t1 = i + 1; t1 < SIZE; ++t1)
            {
                for(int t2 = 0; t2 < j; ++t2)
                {
                    mm[t1 - 1][t2] = m[t1][t2];
                }
                for(int t2 = j + 1; t2 < SIZE; ++t2)
                {
                    mm[t1 - 1][t2 - 1] = m[t1][t2];
                }
            }
            res[i][j] = (((i + j) % 2) * (-2) + 1) * det(mm);
        }
    }
    return res;
}

template<typename TYPE>
mat<TYPE, 3, 3> adjoint(const mat<TYPE, 3, 3>& m)
{
    int x1, y1, x2, y2;
    mat<TYPE, 3, 3> res;
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 3; ++j)
        {
            x1 = (i + 1)%3;
            y1 = (j + 1)%3;
            x2 = (i + 2)%3;
            y2 = (j + 2)%3;
            res[j][i] = m[x1][y1] * m[x2][y2] - m[x1][y2] * m[x2][y1];
        }
    }
    return res;
}

template<typename TYPE>
mat<TYPE, 2, 2> adjoint(const mat<TYPE, 2, 2>& m)
{
    return mat<TYPE, 2, 2>(m[1][1], -m[1][0], -m[0][1], m[0][0]);
}


template<typename TYPE, int SIZE>
void load_identity(mat<TYPE, SIZE, SIZE>& m)
{
    for(int i = 0; i < SIZE; ++i)
    {
        for(int j = 0; j < SIZE; ++j)
        {
            m[i][j] = 0;
        }
    }
    for(int i = 0; i < SIZE; ++i)
    {
        m[i][i] = 1;
    }
}


template<typename TYPE>
mat<TYPE, 4, 4>
make_perspective_mat(float l, float r, float b, float t, float N, float F)
{
    mat<TYPE, 4, 4> m(TYPE(1));
    if( (r == l) || (t == b) || (N == F) || N < TYPE(0) || F < TYPE(0))
    {
        return m;
    }
    m[0][0] = 2.0f * N / (r - l);
    m[1][1] = (2.0f * N) / (t - b);
    m[2][0] = (r + l) / (r - l);// r + l = 0
    m[2][1] = (t + b) / (t - b); //  t + b = 0
    m[2][2] = -((F + N) / (F - N));
    m[2][3] = -1.0f;
    m[3][2] = -(2.0f * F * N / (F - N));
    m[3][3] = 0.0f;
    return m;
}

template<typename TYPE>
mat<TYPE, 4, 4>
make_perspective_mat(float fFov, float fAspect, float N, float F)
{
    float t = N * tanf(fFov * 0.5f);
    float b =  -t;
    float l = b * fAspect;
    float r = -l; 
    return make_perspective_mat<TYPE>(l, r, b, t, N, F);
}


template<typename TYPE>
mat<TYPE, 4, 4>
make_orthographic_mat(float l, float r, float b, float t, float n, float f)
{
    mat<TYPE, 4, 4> m;
    m[0][0] = 2.0f * (r - l);
    m[1][1] = 2.0f * (t - b);
    m[2][2] = -2.0f * (f - n);
    m[3][0] = -((r + l) / (r - l));
    m[3][1] = -((t + b) / (t - b));
    m[3][2] = -((f + n) / (f - n));
    m[3][3] = 1.0f;
    return m;
}


template<typename TYPE>
mat<TYPE, 4, 4> rotation_mat(double angle, double x, double y, double z)
{
    double mag, s, c;
    double xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
    mat<TYPE, 4, 4> m(TYPE(1.0));
    if(x == 0.0 && y == 0 && z == 0)
    {
        return m;
    }
    s = sin(angle);
    c = cos(angle);
    xx = x * x;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    yz = y * z;
    zx = z * x;
    xs = x * s;
    ys = y * s;
    zs = z * s;
    one_c = 1.0 - c;
    m[0][0] = (one_c * xx) + c;
    m[1][0] = (one_c * xy) - zs;
    m[2][0] = (one_c * zx) + ys;

    m[0][1] = (one_c * xy) + zs;
    m[1][1] = (one_c * yy) + c;
    m[2][1] = (one_c * yz) - xs;

    m[0][2] = (one_c * zx) - ys;
    m[1][2] = (one_c * yz) + xs;
    m[2][2] = (one_c * zz) + c;
    return m;
}

template<typename TYPE>
mat<TYPE, 4, 4> translation_mat(double x, double y, double z)
{
    mat<TYPE, 4, 4> m((TYPE)1.0);
    m[3][0] = (TYPE)x;
    m[3][1] = (TYPE)y;
    m[3][2] = (TYPE)z;
    return m;
}

template<typename TYPE>
mat<TYPE, 4, 4> scale_mat(double x, double y, double z)
{
    mat<TYPE, 4, 4> m((TYPE)1.0);
    m[0][0] = (TYPE)x;
    m[1][1] = (TYPE)y;
    m[2][2] = (TYPE)z;
    return m;
}



template<typename TYPE>
mat<TYPE, 4, 4> lookat(const vec<TYPE, 3>& eye, const vec<TYPE, 3>& center, const vec<TYPE, 3>& up)
{

    vec<TYPE, 3> f = (center - eye).normalize();
    vec<TYPE, 3> upN = vec<TYPE, 3> (up).normalize();
    vec<TYPE, 3> s = cross_product(f, upN);
    vec<TYPE, 3> u = cross_product(s, f);
    mat<TYPE, 4, 4> m(
            s[0], u[0], -f[0], TYPE(0),
            s[1], u[1], -f[1], TYPE(0),
            s[2], u[2], -f[2], TYPE(0),
            TYPE(0), TYPE(0), TYPE(0), TYPE(1)
            );
    return m * translation_mat<TYPE>(-eye[0], -eye[1], -eye[2]);
}


}





#endif