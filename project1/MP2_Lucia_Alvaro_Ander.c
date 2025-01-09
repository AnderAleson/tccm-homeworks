#include <stdlib.h>
#include <stdio.h>
#include <trexio.h>
#include <inttypes.h>

// 22-12-2024 16:00 Manage to compile with any error.
// to compile: gcc code_name.c -o code_name -ltrexio
// 22-12-2024 16:45 Manage to read nuclear repulsion.

// 23-12-2024 Manage to compute the one electron integrals (core hamiltonian), value conprobated with an orca calculation

// there is a very similar procedure in https://trex-coe.github.io/trexio/examples.html
// the two electron integral problem is done  in a similar way but in fortran
// 30/12/2024 HF Energy obtained

// 02/01/2025 MP2 Correction done.
// Try and compare with orca calculations to check if similar resultas are obtained.

// 04/01/2025 generate a section for functions.



///////////////////////////   FUNCTIONS  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

//FOR BETTER READABILITY

double fcore_hamiltonian(int number_of_MO, double* core_hamiltonian, int number_of_occupied_orbitals ){
    double E_core_hamiltonian=0.0;
    for (int i = 0; i < number_of_occupied_orbitals; ++i) {
        E_core_hamiltonian=E_core_hamiltonian+ 2*core_hamiltonian[i * number_of_MO + i];   
    }
    return E_core_hamiltonian;
}

double ftwo_electron_contibution(double**** integrals_4d, int number_of_occupied_orbitals){
    double E_two_electron = 0.0;
    
    for (int i = 0; i < number_of_occupied_orbitals; i++) { //JUST RUN OVER THE OCCUPIED ORBITALS
        for (int j = 0; j < number_of_occupied_orbitals; j++) {
            E_two_electron = E_two_electron+ 2.0 * integrals_4d[i][j][i][j] - integrals_4d[i][j][j][i];
        }
    }

    return E_two_electron;
}

double fMP2(int number_of_occupied_orbitals,int number_of_MO, double**** integrals_4d,double* mo_energy ){

    double MP2_energy=0.0; 

       for (int i = 0; i < number_of_occupied_orbitals; i++) {  //RUN OVER THE OCCUPIED ORBITALS
        for (int j = 0; j < number_of_occupied_orbitals; j++) {
            for (int a = number_of_occupied_orbitals; a < number_of_MO; a++){  // RUN OVER VIRTUAL ORBITALS
                for (int b = number_of_occupied_orbitals; b < number_of_MO; b++){
                    
                    MP2_energy=MP2_energy+(integrals_4d[i][j][a][b]*(2*integrals_4d[i][j][a][b]-integrals_4d[i][j][b][a]))/(mo_energy[i]+mo_energy[j]-mo_energy[a]-mo_energy[b]);
                    
                }
            }
           
        }
    }

    return MP2_energy;
}






//////////////////////// END FUNCTIONS ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////





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
        exit(1);
    }

    fprintf(output_file,"               ////////////////////                    \n");
    fprintf(output_file,"               //      MP2       //                    \n");
    fprintf(output_file,"               ////////////////////                    \n");
    fprintf(output_file,"                 %s                             ",__DATE__); // show the date

    fprintf(output_file,"\n                 Ander Aleson.                   \n");
    fprintf(output_file,"                 Lucia Vidal.                    \n");
    fprintf(output_file,"                 Alvaro Serrano.                 \n");
    fprintf(output_file,"\n");

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
    fprintf(output_file,"Nuclear repulsion energy: %f a.u. \n", nuclear_repulsion);
    
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
    fprintf(output_file,"Number of occupied orbitals: %d\n", number_of_occupied_orbitals);
    
    
    
    // Variable to store number of molecular orbitals MO:
    int number_of_MO;
    
    rc = trexio_read_mo_num(trexio_file, &number_of_MO);
    if (rc != TREXIO_SUCCESS) {
		printf("TREXIO Error reading number of Molecular Orbitals:\n%s\n",
		trexio_string_of_error(rc));
		exit(1);
    }
    
    // Print the number of Molecular Orbitals
    
    fprintf(output_file,"Number of molecular orbitals: %d\n", number_of_MO);
    
    //VARAIBLE TO STORE THE NUMBER OF VIRTUAL_ORBITALS
    int number_of_virtual_orbitals;
    
    //GET THE NUMBER OF VIRTUAL ORBITALS
    number_of_virtual_orbitals=number_of_MO - number_of_occupied_orbitals;
    
    fprintf(output_file,"Number of virtual orbitals: %d\n", number_of_virtual_orbitals);
    
    //VARIABLE TO STORE 1 ELECTRON INTEGRALS 
    
    size_t num_one_e_integrals= (size_t) number_of_MO * (size_t) number_of_MO;
    
    double* core_hamiltonian = (double*) malloc(num_one_e_integrals * sizeof(double));
   
    // SHOW ALERT IF MEMORY ALLOCATION FAILS.
    if (core_hamiltonian == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        trexio_close(trexio_file);
        exit(1);
    }
    
    
    // PASS DATA TO THE ALLOCATED MEMORY
    
    rc = trexio_read_mo_1e_int_core_hamiltonian(trexio_file, core_hamiltonian);
    
    if (rc != TREXIO_SUCCESS) {
        fprintf(stderr, "Error reading core Hamiltonian: %s\n", trexio_string_of_error(rc));
        free(core_hamiltonian);
        trexio_close(trexio_file);
        exit(1);
    }
    
    
    // LETS GENERATE A VARIABLE TO STORE THE CORE HAMILTONIAN ENERGY
    
    double E_core_hamiltonian=0.0;
                        // CALL THE FUNCTION TO COMPUTE THE CORE HAMILTONIAN ENERGY
    E_core_hamiltonian=fcore_hamiltonian(number_of_MO,core_hamiltonian, number_of_occupied_orbitals);
    
    // WRITE ONE ELECTRON INTEGRAL ENERGY TO THE OUTPUT.
    fprintf(output_file,"Core hamiltonian energy: %f a.u. \n", E_core_hamiltonian);
    
    
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
	
    //if ALOCATION FAILS SHOW ERROR
	if (value == NULL) {
		fprintf(stderr, "Malloc failed for index");
		exit(1);
	}
    
    int32_t* const index = malloc(4 * number_of_non_zero_integrals * sizeof(int32_t));

    // IF ALOCATION FAILS SHOW ERROR
	if (index == NULL) {
		fprintf(stderr, "Malloc failed for value");
		exit(1);
	}
	
    const int64_t offset_file=0;
    int64_t buffer_size = number_of_non_zero_integrals;
    

    rc = trexio_read_mo_2e_int_eri(trexio_file, offset_file, &buffer_size, index, value);
    
    if (rc != TREXIO_SUCCESS) {
        fprintf(stderr, "Error reading two-electron integral values: %s\n", trexio_string_of_error(rc));
        exit(1);
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

                    //CALL FUNCTION TO COMPUTE THE ENERGY CONTRIBUTION OF TWO ELECTRON INTEGRALS
    E_two_electron=ftwo_electron_contibution(integrals_4d, number_of_occupied_orbitals);


    fprintf(output_file, "Two-electron contribution to HF energy: %f a.u.\n", E_two_electron);

    // Add all contributions to compute the total energy
    total_energy = total_energy +E_two_electron;


    fprintf(output_file, "HF Energy: %f a.u. \n", total_energy);


    //+++++++++++++ MP2 ENERGY CORRECTION ++++++++++++

    // LETS COMPUTE NOW THE MP2 CORRECTION,
    // FOR THIS WE NEED TO EXTRACT THE MO ENERGIES \epsilon
    
    // DEFINE POINTER WHERE MO E. WILL BE STORED
    double* mo_energy = (double*) malloc(number_of_MO * sizeof(double));

    // ALERT FOR ALLOCATION ERROR
    if (mo_energy == NULL) {
        fprintf(stderr, "Memory allocation failed for MO energies.\n");
        trexio_close(trexio_file);
        exit(1);
    }


    // READ THE MO E.

    rc = trexio_read_mo_energy(trexio_file, mo_energy);

    // ALERT FOR ERROR IN READING

    if (rc != TREXIO_SUCCESS) {
        fprintf(stderr, "Error reading MO energies: %s\n", trexio_string_of_error(rc));
        free(mo_energy);
        trexio_close(trexio_file);
        exit(1);
    }

   

    double MP2_energy=0.0;
    MP2_energy=fMP2(number_of_occupied_orbitals,number_of_MO,integrals_4d,mo_energy);

    fprintf(output_file,"MP2 correction Energy: %f  a.u. \n", MP2_energy);
    //ADD MP2 CORRECTION TO THE TOTAL ENERGY
    total_energy=total_energy+MP2_energy;

    fprintf(output_file, "HF Energy with MP2 correction: %f a.u. \n", total_energy);    

    //++++++++++++  FREE ALLOCATED MEMORY +++++++++++++

    // FREE ALL ALLOCATED MEMORY
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

    free(mo_energy);

    //++++++++++++++   CLOSE OPENED FILES  +++++++++++++++++
    
    // CLOSE THE TREXIO FILE
    rc = trexio_close(trexio_file);
    if (rc != TREXIO_SUCCESS) {
        printf("TREXIO Error closing file: %s\n", trexio_string_of_error(rc));
        exit(1);
    }
    //CLOSE OUTPUT FILE
    fclose(output_file);
    
    return 0;
}



