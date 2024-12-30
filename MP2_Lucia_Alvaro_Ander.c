#include <stdlib.h>
#include <stdio.h>
#include <trexio.h>
#include <inttypes.h>

// 22-12-2024 16:00 Manage to compile with any error.
// to compile: gcc code_name.c -o code_name -ltrexio
// 22-12-2024 16:45 Manage to read nuclear repulsion.

// 23-12-2024 Manage to compute the one electron integrals (core hamiltonian), value conprobated with an orca calculation

// there is a very similar procedure in https://trex-coe.github.io/trexio/examples.html
//the problem is done in fortran 30/12/2024
// 30/12/2024 HF Energy obtained



//Lets make that the input Trexio file be introduced as an argument an extra for the output
        // int argc is the counter of arguments introduced
                    // char* argv[] is an array to store as string the arguments
int main(int argc, char* argv[]) {
    
    if (argc < 3) { //here we just make sure that there are atleast 3 arguments (the code and the input file)
        fprintf(stderr, "Usage: %s <filename> <output_filename>\n", argv[0]);  // if not show error
        return EXIT_FAILURE;
    }

    // asign to file name the introduced argument
    const char *filename = argv[1];
    const char *output_filename = argv[2];

    double total_energy;  //HERE WE WILL STORE THE TOTAL ENERGY

    // Open the TREXIO file in read mode
    trexio_exit_code rc;
    trexio_t* trexio_file = trexio_open(filename, 'r', TREXIO_AUTO, &rc);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error: %s\n", trexio_string_of_error(rc));
        exit(1);
    }


    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error: Could not open output file '%s'\n", output_filename);
        return EXIT_FAILURE;
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

    // write the nuclear repulsion energy
    fprintf(output_file,"Nuclear Repulsion Energy: %f a.u. \n ", nuclear_repulsion);
    
    // Variable to store the number of ocuppied orbitals
    int number_of_occupied_orbitals;
    // take into account we are working with closed-shell systems
    rc = trexio_read_electron_up_num(trexio_file, &number_of_occupied_orbitals);
    if (rc != TREXIO_SUCCESS) {
		printf("TREXIO Error reading number of occupied orbitals:\n%s\n",
		trexio_string_of_error(rc));
		exit(1);
    }
 
    // Print the number of ocuppied orbitals.
    fprintf(output_file,"number of occupied orbitals: %d\n", number_of_occupied_orbitals);
    
    
    
    // Variable to store number of molecular orbitals MO:
    int number_of_MO;
    
    rc = trexio_read_mo_num(trexio_file, &number_of_MO);
    if (rc != TREXIO_SUCCESS) {
		printf("TREXIO Error reading number of Molecular Orbitals:\n%s\n",
		trexio_string_of_error(rc));
		exit(1);
    }
    
    // Print the number of Molecular Orbitals
    
    fprintf(output_file,"number of Molecular Orbitals: %d\n", number_of_MO);
    
    //VARAIBLE TO STORE THE NUMBER OF VIRTUAL_ORBITALS
    int number_of_virtual_orbitals;
    
    //GET THE NUMBER OF VIRTUAL ORBITALS
    number_of_virtual_orbitals=number_of_MO - number_of_occupied_orbitals;
    
    fprintf(output_file,"number of Virtual Orbitals: %d\n", number_of_virtual_orbitals);
    
    //VARIABLE TO STORE 1 ELECTRON INTEGRALS +++++++++++++++++++++++++++++++++++++++
    
    size_t num_one_e_integrals= (size_t) number_of_MO * (size_t) number_of_MO;
    
    double* core_hamiltonian = (double*) malloc(num_one_e_integrals * sizeof(double));
   
    // SHOW ALERT IF MEMORY ALLOCATION FAILS.
    
    if (core_hamiltonian == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        trexio_close(trexio_file);
        return EXIT_FAILURE;
    }
    
    
    // PASS DATA TO THE ALLOCATED MEMORY
    
    rc = trexio_read_mo_1e_int_core_hamiltonian(trexio_file, core_hamiltonian);
    
    if (rc != TREXIO_SUCCESS) {
        fprintf(stderr, "Error reading core Hamiltonian: %s\n", trexio_string_of_error(rc));
        free(core_hamiltonian);
        trexio_close(trexio_file);
        return EXIT_FAILURE;
    }
    
    //printf("Core Hamiltonian matrix:\n");
    //for (int i = 0; i < number_of_MO; ++i) {
    //    for (int j = 0; j < number_of_MO; ++j) {
    //        printf("%f ", core_hamiltonian[i * number_of_MO + j]);
    //    }
    //    printf("\n");
    //   
    //}
    
    // LETS GENERATE A VARIABLE TO STORE THE CORE HAMILTONIAN ENERGY
    
    double E_core_hamiltonian=0.0;
    
    for (int i = 0; i < number_of_occupied_orbitals; ++i) {
        E_core_hamiltonian=E_core_hamiltonian+ 2*core_hamiltonian[i * number_of_MO + i];
        
    }
    // SHOW ONE ELECTRON INTEGRAL ENERGY
    
    
    fprintf(output_file,"Core Hamiltonian Energy: %f a.u. \n ", E_core_hamiltonian);
    
    
    // LETS ADD THE CORE HAMILTONIAN ENERGY AND N-N REPULSION ENERGY
    
    total_energy=E_core_hamiltonian+nuclear_repulsion;    
    
    //LETS START WITH THE TWO electron integrals.
    	// SEEMS DATA IS STORED IN SPARSE FORMAT. LETS GET THE NUMBER OF NON ZERO INTEGRALS.
    
    int64_t number_of_non_zero_integrals = 0; 
    rc = trexio_read_mo_2e_int_eri_size(trexio_file, &number_of_non_zero_integrals);
    if (rc != TREXIO_SUCCESS) {
        fprintf(stderr, "Error al leer el tamaño de los integrales: %s\n", trexio_string_of_error(rc));
        trexio_close(trexio_file);
        return 1;
    }

    fprintf(output_file,"Number of non zero integrals: %ld\n", number_of_non_zero_integrals);

	//LETS ALLOCATE THE MEMORY FOR THE INDICES OF THE INTEGRALS and their value
	double* value = (double*) malloc(number_of_non_zero_integrals * sizeof(double));
	
    //if ALOCATION FAIL SHOW
	if (value == NULL) {
		fprintf(stderr, "Malloc failed for index");
		exit(1);
	}
    	int32_t* const index = malloc(4 * number_of_non_zero_integrals * sizeof(int32_t));

    // IF ALOCATION FAIL SHOW
	if (index == NULL) {
		fprintf(stderr, "Malloc failed for value");
		exit(1);
	}
	
    const int64_t offset_file=0;
    int64_t buffer_size = number_of_non_zero_integrals;
    

    rc = trexio_read_mo_2e_int_eri(trexio_file, offset_file, &buffer_size, index, value);
    
    if (rc != TREXIO_SUCCESS) {

        fprintf(stderr, "Error reading two-electron integral values: %s\n", trexio_string_of_error(rc));
        return 1;
    }
		
 /////////////////////////
    // Allocate memory to store the two electron integrals in a 4D array;
    double**** integrals_4d = (double****)malloc(number_of_MO * sizeof(double***));
    for (int i = 0; i < number_of_MO; i++) {
        integrals_4d[i] = (double***)malloc(number_of_MO * sizeof(double**));
        for (int j = 0; j < number_of_MO; j++) {
            integrals_4d[i][j] = (double**)malloc(number_of_MO * sizeof(double*));
            for (int k = 0; k < number_of_MO; k++) {
                integrals_4d[i][j][k] = (double*)calloc(number_of_MO, sizeof(double));
            }
        }
    }

    
    for (int64_t n = 0; n < number_of_non_zero_integrals; n++) {
        int i = index[4 * n];
        int j = index[4 * n + 1];
        int k = index[4 * n + 2];
        int l = index[4 * n + 3];
        double val = value[n];

        // Store the value IN ALL POSIBLE PERMUTATIONS, in the webPAGE EXAMPLE SEEM TO BE A SIMILAR WAY TO EXTRACT
        integrals_4d[i][j][k][l] = val;
        integrals_4d[i][l][k][j] = val;
        integrals_4d[k][l][i][j] = val;
        integrals_4d[k][j][i][l] = val;
        integrals_4d[j][i][l][k] = val;
        integrals_4d[l][i][j][k] = val;
        integrals_4d[l][k][j][i] = val;
        integrals_4d[j][k][l][i] = val;
    }

    // COMPUTE THE COULOMB AND EXCHANGE INTEGRALS

    double E_two_electron = 0.0;


    for (int i = 0; i < number_of_occupied_orbitals; i++) { //JUST RUN OVER THE OCCUPIED ORBITALS
        for (int j = 0; j < number_of_occupied_orbitals; j++) {
            E_two_electron = E_two_electron+ 2.0 * integrals_4d[i][j][i][j] - integrals_4d[i][j][j][i];
        }
    }

    fprintf(output_file, "Two-Electron Contribution to HF Energy: %f a.u.\n", E_two_electron);

    // Add all contributions to compute the total energy
    total_energy = total_energy +E_two_electron;

    fprintf(output_file, "Total HF Energy: %f a.u. \n", total_energy);

    // Free allocated memory
    for (int i = 0; i < number_of_MO; i++) {
        for (int j = 0; j < number_of_MO; j++) {
            for (int k = 0; k < number_of_MO; k++) {
                free(integrals_4d[i][j][k]);
            }
            free(integrals_4d[i][j]);
        }
        free(integrals_4d[i]);
    }
    free(integrals_4d);
    free(index);
    free(value);
    free(core_hamiltonian);

    // Close the TREXIO file
    rc = trexio_close(trexio_file);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error closing file: %s\n", trexio_string_of_error(rc));
        exit(1);
    }
    
    return 0;
}



