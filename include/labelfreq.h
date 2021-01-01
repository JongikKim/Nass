#ifndef __LABEL_FREQ__
#define __LABEL_FREQ__

struct LabelFreq
{
    unsigned* vfreq;
    unsigned* efreq;

    unsigned** vertex_bridges;
    unsigned** vertex_efreq; // not used yet!!

    LabelFreq()
    {
        DataSet* dataset = DataSet::getInstance();

        vfreq = new unsigned[vlabel_map.size()];
        efreq = new unsigned[elabel_map.size()];

        vertex_bridges = new unsigned*[dataset->vmax()];
        for(unsigned i = 0; i < dataset->vmax(); i++){
            vertex_bridges[i] = new unsigned[elabel_map.size()];
            memset(vertex_bridges[i], 0, sizeof(unsigned)*elabel_map.size());
        }

        vertex_efreq = new unsigned*[dataset->vmax()];
        for(unsigned i = 0; i < dataset->vmax(); i++){
            vertex_efreq[i] = new unsigned[elabel_map.size() + 1];
            memset(vertex_efreq[i], 0, sizeof(unsigned)*(elabel_map.size()+1));
        }
    }

    ~LabelFreq()
    {
        delete [] vfreq;
        delete [] efreq;

        DataSet* dataset = DataSet::getInstance();
        for(unsigned i = 0; i < dataset->vmax(); i++)
            delete [] vertex_bridges[i];
        delete [] vertex_bridges;

        for(unsigned i = 0; i < dataset->vmax(); i++)
            delete [] vertex_efreq[i];
        delete [] vertex_efreq;
    }
};

#endif //__LABEL_FREQ__

