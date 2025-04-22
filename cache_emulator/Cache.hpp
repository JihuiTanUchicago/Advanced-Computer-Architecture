#ifndef CACHE_HPP
#define CACHE_HPP
#include "Address.hpp"
#include "DataBlock.hpp"
#include "Ram.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <random>
#include <string>
#include <ctime>

class Cache {
private:
    const int blockSize;
    const int associativity;
    const std::string policy;
    Ram& ram;
    bool useL2;

    /* L1 */
    int l1SetsCnt;
    std::vector<std::list<DataBlock>> l1Sets;
    std::vector<std::unordered_map<int,std::list<DataBlock>::iterator>> l1Map;

    /* L2 */
    static constexpr int L2_SIZE = 131072;
    static constexpr int L2_WAY  = 8;
    int l2SetsCnt = 1;                     // default 1 when L2 off
    std::vector<std::list<DataBlock>> l2Sets;
    std::vector<std::unordered_map<int,std::list<DataBlock>::iterator>> l2Map;

    std::mt19937 rng;
    inline void touch(std::list<DataBlock>& s,
                      std::list<DataBlock>::iterator it)
    { if (policy=="LRU") s.splice(s.begin(),s,it); }

public:
    int l1ReadHits=0,l1ReadMisses=0,l1WriteHits=0,l1WriteMisses=0;
    int l2ReadHits=0,l2ReadMisses=0,l2WriteHits=0,l2WriteMisses=0;

    Cache(int l1Size,int blk,int way,const std::string& pol,
          Ram& r,bool enableL2=false)
      : blockSize(blk),associativity(way),policy(pol),
        ram(r),useL2(enableL2)
    {
        l1SetsCnt=(l1Size/blockSize)/associativity;
        l1Sets.resize(l1SetsCnt); l1Map.resize(l1SetsCnt);

        if(useL2){
            l2SetsCnt=(L2_SIZE/blockSize)/L2_WAY;
            l2Sets.resize(l2SetsCnt); l2Map.resize(l2SetsCnt);
        }
        rng.seed(std::time(nullptr));
    }

    inline int l2Index(const Address& a) const {
        return useL2 ? (a.getRawAddress()/blockSize)%l2SetsCnt : 0;
    }

    double getDouble(const Address& a){
        int tag=a.getTag(), off=a.getOffset()/sizeof(double);
        int i1=a.getIndex();
        auto it1=l1Map[i1].find(tag);
        if(it1!=l1Map[i1].end()){        // L1 hit
            l1ReadHits++; touch(l1Sets[i1],it1->second);
            return it1->second->get(off);
        }
        l1ReadMisses++;

        if(useL2){
            int i2=l2Index(a);
            auto it2=l2Map[i2].find(tag);
            if(it2!=l2Map[i2].end()){    // L2 hit
                l2ReadHits++; touch(l2Sets[i2],it2->second);
                promoteToL1(i1,tag,*it2->second);
                return it2->second->get(off);
            }
            l2ReadMisses++;
        }
        return fetchRAM(a);
    }

    void setDouble(const Address& a,double val){
        int tag=a.getTag(), off=a.getOffset()/sizeof(double);
        int i1=a.getIndex();
        auto it1=l1Map[i1].find(tag);
        if(it1!=l1Map[i1].end()){        // L1 hit
            l1WriteHits++; it1->second->set(off,val);
            ram.setBlock(a,*it1->second); touch(l1Sets[i1],it1->second);
            if(useL2){
                int i2=l2Index(a);
                auto it2=l2Map[i2].find(tag);
                if(it2!=l2Map[i2].end()){
                    l2WriteHits++; it2->second->set(off,val);
                    touch(l2Sets[i2],it2->second);
                }
            }
            return;
        }
        l1WriteMisses++;

        if(useL2){
            int i2=l2Index(a);
            auto it2=l2Map[i2].find(tag);
            if(it2!=l2Map[i2].end()){    // L2 hit
                l2WriteHits++; it2->second->set(off,val);
                ram.setBlock(a,*it2->second);
                promoteToL1(i1,tag,*it2->second);
                touch(l2Sets[i2],it2->second);
                return;
            }
            l2WriteMisses++;
        }

        DataBlock blk(blockSize);
        blk=ram.getBlock(a); blk.setTag(tag); blk.setValid(true);
        blk.set(off,val); ram.setBlock(a,blk);
        insertBoth(i1,l2Index(a),tag,blk);
    }

private:
    double fetchRAM(const Address& a){
        DataBlock blk=ram.getBlock(a);
        blk.setTag(a.getTag()); blk.setValid(true);
        insertBoth(a.getIndex(),l2Index(a),a.getTag(),blk);
        return blk.get(a.getOffset()/sizeof(double));
    }
    void promoteToL1(int i1,int tag,const DataBlock& blk){
        if((int)l1Sets[i1].size()>=associativity) evictL1(i1);
        l1Sets[i1].push_front(blk); l1Map[i1][tag]=l1Sets[i1].begin();
    }
    void insertBoth(int i1,int i2,int tag,const DataBlock& blk){
        if((int)l1Sets[i1].size()>=associativity) evictL1(i1);
        l1Sets[i1].push_front(blk); l1Map[i1][tag]=l1Sets[i1].begin();
        if(useL2){
            if((int)l2Sets[i2].size()>=L2_WAY) evictL2(i2);
            l2Sets[i2].push_front(blk); l2Map[i2][tag]=l2Sets[i2].begin();
        }
    }
    void evictL1(int i1){ auto v=l1Sets[i1].back(); l1Map[i1].erase(v.getTag()); l1Sets[i1].pop_back();}
    void evictL2(int i2){
        auto v=l2Sets[i2].back(); int tag=v.getTag();
        l2Map[i2].erase(tag); l2Sets[i2].pop_back();
        int i1=tag%l1SetsCnt;
        auto it=l1Map[i1].find(tag);
        if(it!=l1Map[i1].end()){ l1Sets[i1].erase(it->second); l1Map[i1].erase(it);}
    }
};

#endif
