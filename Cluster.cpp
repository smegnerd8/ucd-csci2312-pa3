#include <cassert>
#include <sstream>
#include "Cluster.h"
#include "Exceptions.h"
#include "Point.h"
namespace Clustering
{

    unsigned int Cluster::__idGenerator = 0;
    const char POINT_CLUSTER_ID_DELIM = ':';

    
    LNode::LNode(const Point &p, LNodePtr n) : point(p)
    {
        next = n;
    }
    Cluster::Centroid::Centroid(unsigned int d, const Cluster &c) : __dimensions(d), __p(d),__c(c)
    {
        if (__c.__size == 0)
            __valid = false;
        toInfinity();
    }
    
    Cluster::Cluster(unsigned int d) : __dimensionality(d), __size(0),__points(nullptr),centroid(d,*this)
    {
        __id = __idGenerator;
        ++__idGenerator;
    }
    
    const Point Cluster::Centroid::get() const
    {
        
        return __p;
        
    }
    
    void Cluster::Centroid::set(const Point &p)
    {
        
        __valid = true;
        __p = p;
        
    }
    
    bool Cluster::Centroid::isValid() const
    {
        
        return __valid;
        
    }
    
    void Cluster::Centroid::setValid(bool valid)
    {
        
        __valid = valid;
        
    }
    
    void Cluster::Centroid::compute()
    {
        double avgerage;

        if (__c.__size > 0)
         {
            for (unsigned int d = 0; d < __dimensions; ++d)
            {
                avgerage = 0;
                for (unsigned int i = 0; i < __c.__size; ++i)
                {
                    avgerage += (__c[i])[d];
                }
                __p[d] = avgerage / __c.__size;
            }
        }
        else {
            toInfinity();
        }

        __valid = true;

    }
    
    bool Cluster::Centroid::equal(const Point &p) const
    {
        
        bool equal = true;
        
        for (unsigned int i = 0; i < __dimensions; ++i)
        {
            if (__p[i] != p[i])
            {
                equal = true;
                break;
            }
        }
        
        
        return equal;
        
    }
    
    void Cluster::Centroid::toInfinity()
    {
        for (unsigned int i =0; i < __dimensions; ++i)
        {
            __p[i] = std::numeric_limits<double>::max();
            
        }
        
        
        
    }
   void Cluster::pickCentroids(unsigned int k, Point **pointArray)
    {
        LNodePtr curr = __points;
        Point inf(__dimensionality);
        for (int i = 0; i < __dimensionality; ++i)
        {
            inf[i] = std::numeric_limits<double>::max();
        }
        if (k >= __size)
        {
            for (int i = 0; i < __size; ++i)
            {
                *pointArray[i] = curr->point;
                curr = curr->next;
            }
            for (int i = __size; i < k; ++i)
            {
                *pointArray[i] = inf;
            }
        }
        else if (k < __size)
        {
            for (int i = 0; i < k; ++i)
            {
                *pointArray[i] = curr->point;
                curr = curr->next;
            }
        }

    }
    
    
    //Move Class
    
    Cluster::Move::Move(const Point&p, Cluster &from, Cluster&to) :__p(p), __from(from), __to(to)
    {
        
    }
    
    void Cluster::Move::perform()
    {
        
        __to.add(__from.remove(__p));
        
        __to.centroid.setValid(false);
        __from.centroid.setValid(false);

        if(__to.__size == 0)
            __to.centroid.toInfinity();
        if(__from.__size == 0)
            __from.centroid.toInfinity();


    }
    
    
    
    
    //copy constructor for Cluster
    Cluster::Cluster(const Cluster &p) : Cluster(p.__dimensionality)
    {
        
        for (unsigned int i = 0; i <p.getSize(); i ++)
            add(p[i]);

        __id = p.__id;
        centroid.compute();
        
    }
    
    //Destructor
    Cluster::~Cluster()
    {
        LNodePtr d;
        
        while(__points != nullptr) // irrating through the points deleting one at a time until the end 'nullptr'
        {
            d= __points;
            __points = d->next;
            
            delete d;
        }
        
        
    }
    //overload = assignment operator
    Cluster &Cluster::operator=(const Cluster & p)
    {

            LNodePtr cursor;

            cursor = __points;

            while (__points != nullptr) {


                cursor = __points;
                __points = __points->next;

                delete cursor; // freeing up memory :)
                --__size;
            }

        
        
        for (unsigned int i =0; i < p.getSize(); ++i)
            add(p[i]);

        __id = p.__id;
        centroid.compute();
        
        
        return *this;
    }
    
    //getting size of cluster
   unsigned int Cluster::getSize() const {
        
        return __size;
    }
    unsigned int Cluster::getDimensionality() const
    {
        return __dimensionality;
        
    }
    unsigned int Cluster::getId() const
    {
        return __id;
        
    }
    
    void Cluster::add(const Point & p)
    {
        if (__dimensionality != p.getDims())
            throw DimensionalityMismatchEx(__dimensionality, p.getDims());

        LNodePtr nxt;
        LNodePtr prev;

        if (__size == 0) //list is empty
        {
            __points = new LNode(p, nullptr);
            centroid.setValid(false);
            ++__size;
        }
        else //if the list isn't empty
        {

            nxt = __points;
            prev = nullptr;

            centroid.setValid(false);

            while (nxt != nullptr)
            {
                if (p < nxt->point)
                {
                    if (prev == nullptr)
                    {
                        // If first element in the list
                        __points = new LNode(p, nxt);

                        ++__size;

                        return;  // Add
                    }
                    else {
                        // Not the first element in the list
                        prev->next = new LNode(p, nxt);

                        ++__size;

                        return;
                    }
                }

                prev = nxt;
                nxt = nxt->next;

            }

            ++__size;

            prev->next = new LNode(p, nullptr);

        }

    }
    
    const Point &Cluster::remove(const Point &p)
    {
        if(__dimensionality != p.getDims())
            throw DimensionalityMismatchEx(__dimensionality, p.getDims());
        
        LNodePtr curr, prev;
        prev = nullptr;
        curr = __points;
        
        if (contains(p)) //point is in the list.
        {
            centroid.setValid(false);
            while (curr != nullptr)
            {
                if (curr->point == p) //the cursor found the point in the loop.
                {
                    
                    if (prev == nullptr) //Handle case if its the first point in the list.
                    {
                        __points = curr->next;
                        
                        delete curr;
                        
                        --__size;
                        
                        break;
                    }
                    
                    else
                    {                           // to handle case if its not the first point in the list.
                        prev->next = curr->next;
                        delete curr;
                        
                        --__size;
                        
                        break;
                    }

                }
                
                prev = curr;
                curr = curr->next;
            }
            
            
        }
        
        return p;
        
    }
    
    const Point &Cluster::operator[](unsigned int index) const
    {
        if (__size == 0)
            throw EmptyClusterEx();
        
        if (index >= __size)
            throw OutOfBoundsEx(__size,index);
        
        LNodePtr curr = __points;
        
        for (unsigned int i = 0; i < index; i++)
        {
            curr = curr->next;
        }
        
        return curr->point;
        
        
    }
    
    
    bool Cluster::contains(const Point &p) const
    {
        if(__dimensionality != p.getDims())
            throw DimensionalityMismatchEx(__dimensionality, p.getDims());
        
        LNodePtr curr = __points;
        
        while (curr != nullptr)
        {
            
            if (curr->point.getId() == p.getId()) // if Point is in the list. return true else it returns false.
                
                return true;
            
            else
                curr= curr->next;
            
            
        }
        
        return false;
        
    }
    
    //Point Arguments
    
    Cluster &Cluster::operator-=(const Point&p)
    {
        
        remove(p);
        
        return *this;
        
    }
    
    Cluster &Cluster::operator+=(const Point&p)
    {
        add(p);
        
        return *this;
        
    }
    
    //Cluster Arguments
    
    Cluster &Cluster::operator+=(const Cluster &p)
    {
        for(unsigned int i = 0; i < p.getSize(); i++)
        {
            if(!contains(p[i]))
                add(p[i]);
            
        }
        return *this;
    }
    
    
    Cluster &Cluster::operator-=(const Cluster &p)
    {
        
        for(unsigned int i = 0; i <p.__size; i++)
        {
            if(contains(p[i]))
            remove(p[i]);
        }
        
        return *this;
        
        
    }
    
    std::istream &operator>>(std::istream &data, Cluster &p)
    {

//
//        std::string s;
//
//        while(getline(data,s))
//        {
//
//           unsigned int size = (unsigned int)std::count(s.begin(), s.end(), ',');
//            Point in(size +1);
//
//            std::istringstream ss(s);
//
//            try
//            {
//            ss >> in;
//            p.add(in);
//            }
//
//            catch (DimensionalityMismatchEx& e)
//            {
//                std::cerr << "Caught an exception of type: " << e << std::endl;
//                in.rewindIdGen();
//            }
//
//        }

        return data;

    }
    
    std::ostream &operator<<(std::ostream &out, const Cluster & p)
    {
        
        for (int i = 0; i < p.__size; i++)
        {
            out << p[i] << " " << POINT_CLUSTER_ID_DELIM << " " << p.__id << std::endl;

        }


        return out;
    }
    
    //Comparisons
    bool operator==(const Cluster &lhs, const Cluster &rhs)
    {
        
        if (lhs.getSize() != rhs.getSize()) //checking to see if cluster size match for comparison
            return false;
        
        for (unsigned int i = 0; i < lhs.getSize(); i++)
        {
            if (lhs[i] != rhs[i])           //seeing to make sure that each point in the cluser are equal to make compariosn
                return false;
            
            
        }
        
        return true;
        
    }
    
    bool operator !=(const Cluster &lhs, const Cluster &rhs)
    {
        return !(lhs == rhs);
        
    }
    
    //Arithmetic (Cluster and Point)
    const Cluster operator +(const Cluster &lhs, const Point &rhs)
    {
        
        Cluster sum(lhs);
        
        sum += rhs;
        
        return sum;
        
        
    }
    
    
    const Cluster operator - (const Cluster &lhs, const Point &rhs)
    {
        Cluster sum(lhs);
        
        sum -= rhs;
        
        return sum;
        
        
    }
    
    // Arithemtic (Two Clusters)
    const Cluster operator+(const Cluster &lhs, const Cluster &rhs)
    {
        
        Cluster sum(lhs);
        sum += rhs;
        
        return sum;
        
        
    }
    
    const Cluster operator - (const Cluster &lhs, const Cluster &rhs)
    {
        
        Cluster sum(lhs);
        
        sum -= rhs;
        
        return sum;
        
        
    }
    
    
    
}
