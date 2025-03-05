#include <iostream>
#include <mpi.h>
#include <vector>
#include <fstream>
#include <stdexcept>

std::vector<unsigned char> load_in_data(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // Pointer at the end

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    // Get the file size
    std::size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg); // Reset the file pointer to the beginning

    // Example: Read the file contents
    std::vector<unsigned char> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);

    if (!file) {
        throw std::runtime_error("Failed to read file.");
    }

    file.close();
    return data; // Return success
}

int main(int argc, char** argv){

	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::vector<unsigned char> data;
    if(rank == 0){
        try {
            std::string filename = "numbers";
            data = load_in_data(filename);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1; // Return an error code
        }
        // print the data
        for (size_t i = 0; i < data.size();i++){
            std::cout << static_cast<unsigned int>(data[i]) << " ";
        }
        std::cout << std::endl;
    }
    

	std::cout <<  rank << std::endl;
	
	MPI_Finalize();

    return 0;

}
