//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "bin_load.h"

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------
bin_load::bin_load(const char *filename, mem_api *target)
{
    m_filename    = std::string(filename);
    m_target      = target;
}
//-----------------------------------------------------------------
// load: Binary load
//-----------------------------------------------------------------
bool bin_load::load(uint32_t mem_base, uint32_t mem_size)
{
    // Load file
    FILE *f = fopen(m_filename.c_str(), "rb");
    if (f)
    {
        long size;
        char *buf;
        int error = 1;

        // Get size
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        rewind(f);

        buf = (char*)malloc(size+1);
        if (buf)
        {
            // Read file data in
            int len = fread(buf, 1, size, f);
            buf[len] = 0;

            if (!m_target->create_memory(mem_base, mem_size))
                fprintf (stderr,"Error: Could not allocate memory\n");
            else
            {
                error = 0;
                for (int i=0;i<len;i++)
                {
                    if (m_target->valid_addr(mem_base + i))
                        m_target->write(mem_base + i, buf[i]);
                    else
                    {
                        fprintf (stderr,"Error: Could not load image to memory\n");
                        error = 1;
                        break;
                    }
                }
            }

            free(buf);
            fclose(f);
        }

        return !error;
    }
    else
    {
        fprintf (stderr,"Error: Could not open %s\n", m_filename.c_str());
        return false;
    }
}