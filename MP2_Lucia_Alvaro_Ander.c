#include <stdlib.h>
#include <stdio.h>
#include <trexio.h>

// 22-12-2024 16:00 Manage to compile with any error.
// to compile: gcc code_name.c -o code_name -ltrexio
// 22-12-2024 16:45 Manage to read nuclear repulsion.


int main() {
    // Define the filename of the TREXIO file
    const char *filename = "h2o.h5";
    double total_energy  //HERE WE WILL STORE THE TOTAL ENERGY

    // Open the TREXIO file in read mode
    trexio_exit_code rc;
    trexio_t* trexio_file = trexio_open(filename, 'r', TREXIO_AUTO, &rc);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error: %s\n", trexio_string_of_error(rc));
        exit(1);
    }

    // Variable to store the nuclear repulsion energy E_NN
    double nuclear_repulsion;

    // Read the nuclear repulsion energy from the TREXIO file
    rc = trexio_read_nucleus_repulsion(trexio_file, &nuclear_repulsion);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error reading nuclear repulsion: %s\n", trexio_string_of_error(rc));
        trexio_close(trexio_file);
        exit(1);
    }

    // Print the nuclear repulsion energy
    printf("Nuclear Repulsion Energy: %f\n", nuclear_repulsion);
    
    // Variable to store the number of ocuppied orbitals
    int number_of_occupied_orbitals;
    
    rc = trexio_read_electron_up_num(trexio_file, &number_of_occupied_orbitals);
    if (rc != TREXIO_SUCCESS) {
		printf("TREXIO Error reading number of occupied orbitals:\n%s\n",
		trexio_string_of_error(rc));
		exit(1);
    }
 
    // Print the nuclear repulsion energy
    printf("number of occupied orbitals: %d\n", number_of_occupied_orbitals);
    
    
    
    
    
    
    
    

    // Close the TREXIO file
    rc = trexio_close(trexio_file);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error closing file: %s\n", trexio_string_of_error(rc));
        exit(1);
    }
    
    
    
    
    

    return 0;
}

