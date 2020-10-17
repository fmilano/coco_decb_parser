#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>


int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        std::cerr << "No input file" << std::endl;


        std::cout << "TRS-80 Coco DECB parser." << std::endl << "Usage: " << argv[0] << " ml.bin" << std::endl;

        return 1;
    }

    std::string inputFile{argv[1]};

    std::ifstream ifile(inputFile, std::ios::binary);
    if (!ifile.is_open())
    {
        std::cerr << "Cannot open file " << inputFile << std::endl;
        return 1;    
    }

    ifile.seekg(0, std::ios_base::end);
    auto length = ifile.tellg();
    ifile.seekg(0, std::ios_base::beg); 

    std::vector<unsigned char> input;
    input.resize(static_cast<size_t>(length)); 

    ifile.read(reinterpret_cast<char*>(input.data()), length); 

    auto success = !ifile.fail() && length == ifile.gcount(); 

    ifile.close();

    if (!success)
    {
        std::cerr << "Cannot read file " << inputFile << std::endl;
        return 1;    

    }

    // Parse the input file. Parse states.

    enum class State {
        PREAMBLE_MAGIC_NUMBER,
        PREAMBLE_LEN_1,
        PREAMBLE_LEN_2,
        PREAMBLE_ADDR_1,
        PREAMBLE_ADDR_2,
        DATA,
        PREAMBLE_OR_POSTAMBLE_MAGIC_NUMBER,
        POSTAMBLE_ZERO_1,
        POSTAMBLE_ZERO_2,
        POSTAMBLE_EXEC_ADDR_1,
        POSTAMBLE_EXEC_ADDR_2,
        END
    };


    State state = State::PREAMBLE_MAGIC_NUMBER;

    int parsedSections = 0;

    size_t sectionLen;
    size_t beginAddr;
    int endAddr;
    int execAddr;

    size_t dataCount;

    std::ofstream ofile;

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (State::PREAMBLE_MAGIC_NUMBER == state)
        {
            if (input[i] != 0)
            {
                std::cerr << "Invalid first preamble magic number. Parsing aborted." << std::endl;
                return 1;
            }

            state = State::PREAMBLE_LEN_1;

        }
        else if (State::PREAMBLE_LEN_1 == state)
        {
            sectionLen = input[i];
            sectionLen <<= 8;

            state = State::PREAMBLE_LEN_2;
        }
        else if (State::PREAMBLE_LEN_2 == state)
        {
            sectionLen += input[i];

            state = State::PREAMBLE_ADDR_1; 
        }
        else if (State::PREAMBLE_ADDR_1 == state)
        {
            beginAddr = input[i];
            beginAddr <<= 8;

            state = State::PREAMBLE_ADDR_2;
        }
        else if (State::PREAMBLE_ADDR_2 == state)
        {
            beginAddr += input[i];

            dataCount = 0;
            state = State::DATA;

            ++parsedSections;

            std::cout << "Section: " << parsedSections << std::endl;
            std::cout << "Address: 0x" << std::hex << beginAddr << std::dec << " (" << beginAddr << ")" << std::endl;
            std::cout << "Length: " << sectionLen << " bytes" << std::endl;
            std::cout << "------------------------" << std::endl;

        }
        else if (State::DATA == state)
        {
            if (0 == dataCount)
            {
                size_t lastindex = inputFile.find_last_of(".");

                std::string rawname = inputFile;
                if (lastindex != std::string::npos)
                    rawname = inputFile.substr(0, lastindex);

                std::stringstream ss; 
                ss << rawname << "_" << std::hex << std::setfill('0') << std::setw(4) << beginAddr << ".bin";

                ofile.open(ss.str(), std::ios::binary);
                if (!ofile.is_open())
                {
                    std::cerr << "Could not open output file " << ss.str() << ". Parsing aborted." << std::endl;
                    return 1;
                }
            }

            ofile.put(input[i]);
            ++dataCount;

            if (dataCount >= sectionLen)
            {
                ofile.close();
                state = State::PREAMBLE_OR_POSTAMBLE_MAGIC_NUMBER;
            }
        }
        else if (State::PREAMBLE_OR_POSTAMBLE_MAGIC_NUMBER == state)
        {
            if (0 == input[i])
            {
                state = State::PREAMBLE_LEN_1;
            }
            else if (0xFF == input[i])
            {
                state = State::POSTAMBLE_ZERO_1;
            }
            else
            {
                std::cerr << "Preamble/postamble magic number not found. Parsing aborted." << std::endl;
                return 1;
            }            
        }
        else if (State::POSTAMBLE_ZERO_1 == state)
        {
            if (input[i] != 0x00)
                std::cout << "Warning! Postable zero 1 is not 0." << std::endl;

            state = State::POSTAMBLE_ZERO_2;
        }
        else if (State::POSTAMBLE_ZERO_2 == state)
        {
            if (input[i] != 0x00)
                std::cout << "Warning! Postable zero 2 is not 0." << std::endl;
                
            state = State::POSTAMBLE_EXEC_ADDR_1;
        }
        else if (State::POSTAMBLE_EXEC_ADDR_1 == state)
        {
            execAddr = input[i];
            execAddr <<= 8;

            state = State::POSTAMBLE_EXEC_ADDR_2;

        }
        else if (State::POSTAMBLE_EXEC_ADDR_2 == state)
        {
            execAddr += input[i];
            state = State::END;
        }
        else
        {
            std::cerr << "Invalid parser state." << std::endl;
            return 1;
        }
        
    }

    if (state != State::END)
    {
        std::cerr << "Invalid input file." << std::endl;
        return 1;
    }

    std::cout << "Execution address: " << std::hex << execAddr << std::dec << " (" << execAddr << ")" << std::endl;
            
    std::cout << "Total sections: " << parsedSections << std::endl;


    return 0;
}
