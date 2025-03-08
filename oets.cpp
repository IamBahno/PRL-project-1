/*
 * Projekt: Projekt 1 do predmetu PRL - Odd-even trannsposition sort
 * Autor: Ondřej Bahounek, xbahou00@stud.fit.vutbr.cz
 * Datum: 6.3.2025
 */

#include <iostream>
#include <mpi.h>
#include <fstream>
#include <stdexcept>

unsigned char* load_in_data(const std::string& filename,std::size_t& fileSize) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // Pointer at the end of the file

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    // Get the file size
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg); // Reset the file pointer to the beginning

    // Read the file contents
    unsigned char* data = (unsigned char*)malloc(fileSize);
    if (!data) {
        throw std::runtime_error("Memory allocation failed.");
    }

    // Read in fileSize number of bytes and cast it to unsigned char
    file.read(reinterpret_cast<char*>(data), fileSize);

    if (!file) {
        throw std::runtime_error("Failed to read file.");
    }

    file.close();
    return data;
}

void odd_even_sort(unsigned char* data_point, int rank, int size, MPI_Comm comm){
    // Instead of N/2 iteration containing both odd and even shift
    // I implemented N iteration switching between odd and even for reducing code duplication
    for(int phase = 1; phase <= size; phase ++){ //starting at one, so we can start with odd phase
        int partner; // rank of the partner with whom you swap data this iteration
        unsigned char partners_data;
        unsigned char tmp;
        if(phase % 2 == 1){ // odd phase
            if(rank % 2 == 0){ // even element
                partner = rank + 1;
            }
            else{ // odd element
                partner = rank - 1;
            }
        }
        else{ // even phase
            if(rank % 2 == 0){ //even element
                partner = rank - 1;
            }
            else{ //odd element
                partner = rank + 1;
            }
        }

        if(partner >= 0 && partner < size){ // skip edge cases
            // the element closer to left, contacts the second elemnt
            if(rank < partner){
                // send my value to the partner
                MPI_Send(data_point, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                // save whatever partner sends (either his value if we are switching or mine back)
                MPI_Recv(data_point, 1, MPI_UNSIGNED_CHAR, partner, 0, comm, MPI_STATUS_IGNORE);
            } else{
                // recieve data fro maprther
                MPI_Recv(&partners_data, 1, MPI_UNSIGNED_CHAR, partner, 0, comm, MPI_STATUS_IGNORE);
                if(partners_data > *data_point){ // swithching
                    // send my value to the partner
                    tmp = *data_point; // using tmp variable so i dont have to deal with overwriting values or smthing...
                    MPI_Send(&tmp, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                    // save the recieved value
                    *data_point = partners_data;
                } else{
                    // send back his old value
                    MPI_Send(&partners_data, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                }
            }
        }
        // sync after each iteration
        MPI_Barrier(comm);

    }
    return;
}


int main(int argc, char** argv){

	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    unsigned char* data = nullptr;
    std::size_t data_size = 0;

    // rank 0 will read in data and print it
    if(rank == 0){
        try {
            std::string filename = "numbers";
            data = load_in_data(filename,data_size);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // print the data
        for (size_t i = 0; i < data_size;i++){
            std::cout << static_cast<unsigned int>(data[i]) << " ";
        }
        std::cout << std::endl;
    }
    unsigned char data_point; // variable for storing individual values assigned to each rank
    // distribute values between individual processes
    MPI_Scatter(data, 1, MPI_UNSIGNED_CHAR, &data_point, 1, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // sort
    odd_even_sort(&data_point,rank,size,MPI_COMM_WORLD);

    // gather data from processes back together
    MPI_Gather(&data_point, 1, MPI_UNSIGNED_CHAR, data, 1, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Print the sorted data
    if(rank == 0){
        for(size_t i = 0; i < size; i++){
            std::cout << static_cast<unsigned int>(data[i]) << std::endl;
        }
    }
    // free array
    free(data);

	MPI_Finalize();

    return 0;

}
