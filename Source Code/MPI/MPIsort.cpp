#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <boost/mpi.hpp>
#include <boost/algorithm/string.hpp> 
#include <boost/serialization/map.hpp>

using namespace std;
namespace mpi=::boost::mpi;

void print_help()
{
    cout << "Usage: mpiexec word-count <path to file>"<< endl;
}

char separator[] = " ,.\n-+;:!?()\t[]{}<>'`\"";

bool isseparator (char c) { 
    char* e = separator + sizeof(separator) / sizeof(separator[0]);            
    char* pos = std::find(separator, e, c);
    return (pos != e); 
}

struct sort_reverse {
    bool operator()(const std::pair<string,int> &left, const std::pair<string,int> &right) {
        return left.second > right.second;
    }
};

int main(int argc, char *argv[])
{
    mpi::environment env(argc, argv);
    mpi::communicator world;

    if (argc != 2)
    {
        if (world.rank() == 0)
        {
            print_help();
        }
        return 1;
    }
    
    int chunksize;
    int mapnodescount = world.size() - 1;
    int masterrank = 0;
    int chunk_sizemax;
    unsigned int maxoutputlines = 25;

    //masterrank vars
    char* buf;
    int buf_len;
    vector<map<string, unsigned long long int>> stats;
    map<string, unsigned long long int> result;

    //slaves vars
    map<string, unsigned long long int> stat;
    // vector<string, unsigned long long int> stat_plain;
    char* input = NULL;

    if (world.rank() == masterrank)
    {
        // read input file
        string word;
        ifstream infile(argv[1]);
        
        infile.seekg(0, ios::end);    
        buf_len = infile.tellg();  
        infile.seekg(0, ios::beg);  
        buf = new char[buf_len];   
        infile.read(buf, buf_len); 
        infile.close(); 

        chunksize = buf_len / mapnodescount;
        chunk_sizemax = chunksize * 2;

        mpi::broadcast(world, chunk_sizemax, masterrank);

        // split in chunks and send    
        int i, start_index = 0;
        for (i = 1; i < mapnodescount; i ++){
            if (start_index >= buf_len)
            {
                world.send(i, 0, buf + buf_len, 1);
            }
            else
            {
                int size = chunksize;
                while (size < chunk_sizemax && start_index + size <= buf_len && 
                       !isseparator(buf[start_index + size - 1]))
                { 
                    size++;
                }
                buf[start_index + size - 1] = 0;
                world.send(i, 0, buf + start_index, max(size, 1));
                start_index += size;
            }
        }
        world.send(i, 0, buf + min(start_index, buf_len), max(buf_len - start_index + 1, 1));
    }
    else {
        // get inputs
        mpi::broadcast(world, chunk_sizemax, masterrank);
        input = new char[chunk_sizemax];
        world.recv(masterrank, 0, input, chunk_sizemax);
        //cout << "worker #" << world.rank() << ": '" << input << "'" << endl; //DEBUG
    }
    //barrier and timing
    world.barrier();
    mpi::timer timer;

    // do word count
    if(world.rank() != masterrank) {
        char * word;
        word = strtok(input,separator);
        while (word != NULL)
        {
            string s_word(word);
            boost::algorithm::to_lower(s_word);
            stat[s_word] += 1;
            word = strtok (NULL, separator);
        }
        vector<string> smth;
        world.send(masterrank, 1, smth);
    }

    // collect results
    mpi::gather(world, stat, stats, masterrank);

    if(world.rank() == masterrank)
    {
        for(vector<map<string, unsigned long long int>>::iterator it = stats.begin(); it != stats.end(); ++it) {
            for (map<string, unsigned long long int>::iterator vit = it->begin(); vit != it->end(); ++vit) {
                result[vit->first] += vit->second;
            }            
        }
    }

    // output sorted results
    if (world.rank() == masterrank) 
    {
        cout << "Time: " << timer.elapsed() << "s" << endl;
        cout << "Total words: " << result.size() << endl;
        vector<pair<string, unsigned long long int>> output;
        for (map<string, unsigned long long int>::iterator it = result.begin(); it != result.end(); ++it) {
            output.push_back(make_pair(it->first, it->second));
        }
        sort(output.begin(), output.end(), sort_reverse());
        for (unsigned int i = 0; i < output.size() && i < maxoutputlines; i++) {
            cout << output[i].first << " => " << output[i].second << endl;
        }
    }
    return 0;
}
