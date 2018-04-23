#ifndef SP2_MATRIX_IMPL_H
#define SP2_MATRIX_IMPL_H

namespace sp {

template<typename T, int D> class Matrix
{
public:
    T data[D*D];
    
    Matrix()
    {
    }

    Matrix operator*(const Matrix& m) const
    {
        Matrix ret;
        for(unsigned int x=0; x<D; x++)
        {
            for(unsigned int y=0; y<D; y++)
            {
                T value = 0;
                for(unsigned int n=0; n<D; n++)
                {
                    value += data[x + n * D] * m.data[n + y * D];
                }
                ret.data[x + y * D] = value;
            }
        }
        return ret;
    }
};

};//namespace sp

#endif//MATRIX_IMPL_H
