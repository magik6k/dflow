/*
Copyright (c) 2013 Lukasz Magiera

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <chrono>

using namespace std;

string si64(int64_t in)
{
    string tmp, ret;
    if (in < 0)
    {
        ret = "-";
        in = -in;
    }
    do
    {
        tmp += in % 10 + 48;
        in -= in % 10;
    }
    while (in /= 10);
    for (int i = tmp.size() - 1; i >= 0; i--)
        ret += tmp[i];

    return ret;
}

string sizestr(int64_t in)
{
    bool m = false;
    if(in<0){in = -in;m = true;}
        float tcn = 0;
        string end;
        if (in >= 1099511627776LL)
        {
                tcn = double(in) / 1024.f / 1024.f / 1024.f / 1024.f;
                end = " TiB";
        }
        else if (in >= 1073741824LL)
        {
                tcn = double(in) / 1024.f / 1024.f / 1024.f;
                end = " GiB";
        }
        else if (in >= 1048576LL)
        {
                tcn = double(in) / 1024.f / 1024.f;
                end = " MiB";
        }
        else if (in >= 1024LL)
        {
                tcn = double(in) / 1024.f;
                end = " KiB";
        }
        else
        {
                tcn = in;
                end = " B";
        }

        string ret = si64(tcn);
        if (int(100.f * (tcn - float(int(tcn)))))
        {
                ret += ".";
                ret += si64(int(100.f * (tcn - float(int(tcn)))));
        }
        ret += end;

        return m?("-"+ret):ret;
}

int main(int argc, char *argv[])
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    auto start = std::chrono::high_resolution_clock::now();
    auto last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start).count()-500;

    int64_t dump = -1;
    size_t buf_size = 1;
    void* buf = nullptr;
    bool run = true;
    int64_t percentDiv = 0;
    freopen(NULL, "rb", stdin);
    freopen(NULL, "wb", stdout);

    int sino = fileno(stdin);

    ///Counters:
    int64_t total = 0;
    int64_t lst_rate = 0;

    for(int i = 1; i<argc; ++i)
    {
        if(strlen(argv[i])<2||argv[i][0]!='-')
        {
            cerr << "Invalid argunent: " << argv[i] << endl;
            cerr << "use dflow -h to get help" << endl;
            return 1;
        }
        switch(argv[i][1])
        {
            case 'h':
                cerr << "dflow by Lukasz Magiera(magik6k at gmail.com)" << endl;
                cerr << "Usage: dflow [-d] [-h]" << endl;
                cerr << "Options:" << endl;
                cerr << " -h           Show this help" << endl;
                cerr << " -d           Do not output data" << endl;
                cerr << " -l [number]  Stop outputing data when specified amount is reached" << endl;
                cerr << " -b [number]  Set buffer size to number" << endl;
                cerr << " -p [number]  Count percent of transfered data - number" << endl;
                break;
            case 'd':
                dump = 0;
                break;
            case 'l':
                ++i;
                buf_size = stoll(argv[i]);
                break;
            case 'b':
                ++i;
                buf_size = stoll(argv[i]);
                break;
            case 'p':
                ++i;
                percentDiv = stoll(argv[i]);
                break;
            default:
                cerr << "Unknown argument " << argv[i] << endl;
                cerr << "use dflow -h to get help" << endl;
                break;
        }
    }

    buf = new char[buf_size];

    //main loop

    cerr << " ";

    while(run)
    {
        size_t rres = 0;
        do
        {
            //rres = fread(buf, buf_size, 1, stdin);
            rres = read(sino,buf,buf_size);
        }
        while (rres < 0 && errno == EINTR);

        if(rres == 0)
        {
            cerr << endl << "[IN]EOF reached" << endl;
            return 0;
        }

        if(rres < 0)
        {
            cerr << "[IN]An error occured" << endl;
            return 2;
        }

        size_t wres = 0;

        if(dump < 0 || dump > total)
        {
            do
            {
                wres += fwrite(buf, rres, 1, stdout) * rres;
                total += wres;
                lst_rate += wres;
                if(ferror(stdout))
                {
                    cerr << "[OUT]An error occured" << endl;
                    return 2;
                }
            }while(wres < rres);
        }
        else
        {
            total += rres;
            lst_rate += rres;
        }

        if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start).count()-last > 500)
        {
            last = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start).count();
            cerr << "\rTotal: " << sizestr(total) << ", Rate: " << sizestr(lst_rate * (1000/500)) << "/s";
            if(percentDiv > 0) cerr << ", " << ((total*100)/percentDiv) << "%";
            cerr << "                     \r" << std::flush;
            lst_rate = 0;
        }
    }

    delete[] (char*)buf;

    return 0;
}
