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
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // Pointer at the end

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    // Get the file size
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg); // Reset the file pointer to the beginning

    // Example: Read the file contents
    unsigned char* data = (unsigned char*)malloc(fileSize);
    if (!data) {
        throw std::runtime_error("Memory allocation failed.");
    }

    file.read(reinterpret_cast<char*>(data), fileSize);

    if (!file) {
        throw std::runtime_error("Failed to read file.");
    }

    file.close();
    return data; // Return success
}

void odd_even_sort(unsigned char* data_point, int rank, int size, MPI_Comm comm){
    for(int phase = 1; phase <= size; phase ++){ //starting at one, so we can start with odd phase
        int partner;
        unsigned char partners_data;
        unsigned char tmp;
        if(phase % 2 == 1){ // odd phase
            if(rank % 2 == 0){ // even element
                partner = rank + 1;
            }
            else{
                partner = rank - 1;
            }
        }
        else{ // even phase
            if(rank % 2 == 0){ //even element
                partner = rank - 1;
            }
            else{
                partner = rank + 1;
            }
        }

        if(partner >= 0 && partner < size){ // skip edge cases
            // the element closer to left, contacts the second elemnt
            if(rank < partner){
                MPI_Send(data_point, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                // save whatever partner sends
                MPI_Recv(data_point, 1, MPI_UNSIGNED_CHAR, partner, 0, comm, MPI_STATUS_IGNORE);
            } else{
                MPI_Recv(&partners_data, 1, MPI_UNSIGNED_CHAR, partner, 0, comm, MPI_STATUS_IGNORE);
                if(partners_data > *data_point){
                    // send my value to the partner
                    tmp = *data_point;
                    MPI_Send(&tmp, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                    // save the recieved value
                    *data_point = partners_data;
                } else{
                    // send back his old value
                    MPI_Send(&partners_data, 1, MPI_UNSIGNED_CHAR, partner, 0, comm);
                }
            }
        }

        MPI_Barrier(comm);

    }
    return;
}

//TODO okomentovat
//TODO mozna zmenit ze misto N iteraci kde se strida odd a even faze na, N/2 iteraci kde se udela vzdy even aji odd
//Odevzdat 

int main(int argc, char** argv){

	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    unsigned char* data = nullptr;
    std::size_t data_size = 0;
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
    unsigned char data_point;
    MPI_Scatter(data, 1, MPI_UNSIGNED_CHAR, &data_point, 1, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    odd_even_sort(&data_point,rank,size,MPI_COMM_WORLD);
    
    MPI_Gather(&data_point, 1, MPI_UNSIGNED_CHAR, data, 1, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if(rank == 0){
        for(size_t i = 0; i < size; i++){
            std::cout << static_cast<unsigned int>(data[i]) << std::endl;
        }
    }
    free(data);

	MPI_Finalize();

    return 0;

}
