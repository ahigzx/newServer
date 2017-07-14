#ifndef BASE_H
#define BASE_H

class noncopyable
{
    public:
        noncopyable(){

        }
        ~noncopyable(){

        }
    private:
        noncopyable& operator =(const noncopyable&);
        noncopyable(const noncopyable&);
};
#endif
