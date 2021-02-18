#pragma once

#include <shared/DistanceMatrix.h>
#include <shared/Matrix.h>
#include <shared/path_utils/PermutationPath.h>

#include <algorithm>  // std::fill_n
#include <chrono>     // std::chrono::milliseconds
#include <memory>     // std::unique_ptr, std::make_unique
#include <optional>   // std::optional
#include <queue>      // std::deque
#include <sstream>    // std::stringstream
#include <thread>     // std::thread::hardware_concurrency
#include <utility>    // std::pair
#include <vector>     // std::vector

#include "cpx_macro.h"

// error status and messagge buffer
int status;
char errmsg[BUF_SIZE];

template <typename T>
class CPLEXModel {
    // CPLEX environment
    Env env;

    // CPLEX linear programming problem
    Prob lp;

    // cost matrix for the N points
    DistanceMatrix<T>& distance_matrix;

    // number of points
    const int N;

    // expected number of variables in the model
    const size_t n_variables;

    // pointer to map for y variables, used to retrieve the solution path
    std::unique_ptr<Matrix<int>> y_variable_mat = nullptr;

    // Setup the linear programming problem
    void setup_lp() noexcept(false);

    // Add a new column (variable) to the CPLEX environment.
    // variable: letter of the variable being added.
    // type: type of the variable. 'C' for continuous variables, 'B' for {0, 1} variables.
    // lower_bound: lower bound of the variable.
    // upper_bound: upper bound of the variable.
    // ij: indexes of the variable.
    // objective_coefficient: objective function coefficient of the new variable.
    void add_column(char variable, char type, double lower_bound, double upper_bound,
                    std::pair<size_t, size_t> ij, T objective_coefficient = 0) noexcept(false);

    // Tell CPLEX to use the parallel opportunistic mode
    void setup_parallelism() noexcept(false);

    // Set the maximum allotted time for computation to the CPLEX environment
    void force_time_limit(const std::chrono::milliseconds& timeout_ms) noexcept(false);

    // Retrieve the Hamiltonian path of the solution
    std::vector<size_t> retrieve_path(std::vector<double>&& variable_values) const noexcept;

public:
    CPLEXModel(DistanceMatrix<T>& distance_matrix,
               const std::chrono::milliseconds& timeout_ms) noexcept(false);

    // Release CPLEX resources
    ~CPLEXModel() noexcept(false);

    void solve();

    // Retrieve the solution cost of the linear problem.
    // It must be called after the "solve" method.
    std::optional<PermutationPath<T>> get_solution() const;
};

template <typename T>
inline CPLEXModel<T>::CPLEXModel(DistanceMatrix<T>& distance_matrix,
                                 const std::chrono::milliseconds& timeout_ms) noexcept(false) :
    distance_matrix(distance_matrix),
    N(static_cast<int>(distance_matrix.size())),
    n_variables((this->N - 1) * (2 * this->N - 1)) {
    // CPLEX declarations
    DECL_ENV(env);
    DECL_PROB(env, lp);

    setup_parallelism();
    force_time_limit(timeout_ms);
    setup_lp();
}

// Release CPLEX resources
template <typename T>
inline CPLEXModel<T>::~CPLEXModel() noexcept(false) {
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
}

// Setup the linear programming problem
template <typename T>
inline void CPLEXModel<T>::setup_lp() noexcept(false) {
    /**
     * Decision variable declarations.
     * Since the variables must be referenced in the constraint declaration section,
     * their position in CPLEX is stored into a square matrix.
     * The position at Matrix<T>::at(i, j) identifies the variable position at indexes i, j.
     */

    /**
     * Equation (6)
     * $x_{i,j} \in [0, N-1]$
     */

    // x_position starts from the number of variables in the LP problem object
    int x_position = CPXgetnumcols(env, lp);
    Matrix<int> x_variable_mat(this->N, this->N);

    for (int i = 0; i < this->N; ++i) {
        for (int j = 1; j < this->N; ++j) {
            if (i != j) {
                add_column('x', 'C', 0, this->N - 1, {i, j});
                x_variable_mat.at(i, j) = x_position++;
            }
        }
    }

    /**
     * Equation (7)
     * $y_{i,j} \in \{ 0, 1 \}$
     */

    // y_position starts from the number of variables in the LP problem object
    int y_position = CPXgetnumcols(env, lp);
    this->y_variable_mat = std::make_unique<Matrix<int>>(this->N, this->N);

    for (int i = 0; i < this->N; ++i) {
        for (int j = 0; j < this->N; ++j) {
            if (i != j) {
                T objective_coefficient = this->distance_matrix.at(i, j);
                add_column('y', 'B', 0, 1, {i, j}, objective_coefficient);
                this->y_variable_mat->at(i, j) = y_position++;
            }
        }
    }

    /**
     * Constraints
     */

    /**
     * Equation (2)
     * Flow in - Flow out = 1
     */

    const int sum1 = this->N - 1;
    const int sum2 = this->N - 2;

    for (int k = 1; k < this->N; ++k) {
        std::vector<int> left_side(sum1 + sum2);
        std::vector<double> coeff(sum1 + sum2);

        // the N-1 $x_ik$ variables have coefficient 1
        std::fill_n(coeff.begin(), sum1, 1.0);

        // the N-2 $x_kj$ variables have coefficient -1
        std::fill_n(std::next(coeff.begin(), sum1), sum2, -1.0);

        // right side: `= 1`
        char sense = 'E';
        const double right_side = 1.0;

        // $x_{ik}$
        int idx = 0;
        for (int i = 0; i < this->N; ++i) {
            if (i != k) {
                left_side[idx++] = x_variable_mat.at(i, k);
            }
        }

        // $x_{kj}, j \neq 0$
        for (int j = 1; j < this->N; ++j) {
            if (j != k) {
                left_side[idx++] = x_variable_mat.at(k, j);
            }
        }

        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, sum1 + sum2, &right_side, &sense, &matbeg,
                         left_side.data(), coeff.data(), nullptr, nullptr);
    }

    /**
     * Equation (3)
     * $sum_{j:(i,j) in A} y_ij = 1 \forall i \in N$
     */

    for (int i = 0; i < this->N; ++i) {
        std::vector<int> left_size(this->N - 1);

        // all coefficients have value 1
        std::vector<double> coeff(this->N - 1, 1.0);

        // right side: `= 1`
        char sense = 'E';
        const double right_side = 1.0;

        // Populate left size
        int idx = 0;
        for (int j = 0; j < this->N; ++j) {
            if (i != j) {
                left_size[idx++] = this->y_variable_mat->at(i, j);
            }
        }

        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, this->N - 1, &right_side, &sense, &matbeg,
                         left_size.data(), coeff.data(), nullptr, nullptr);
    }

    /**
     * Equation (4)
     * $sum_{i:(i,j) in A} y_ij = 1 \forall j \in N$
     */

    for (int j = 0; j < this->N; ++j) {
        std::vector<int> left_size(this->N - 1);
        std::vector<double> coeff(this->N - 1);
        std::fill_n(coeff.begin(), this->N - 1, 1.0);  // All coefficients are 1.0

        // right side: `= 1`
        char sense = 'E';
        const double right_side = 1.0;

        // Populate left size
        int idx = 0;
        for (int i = 0; i < this->N; ++i) {
            if (i != j) {
                left_size[idx++] = this->y_variable_mat->at(i, j);
            }
        }

        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, this->N - 1, &right_side, &sense, &matbeg,
                         left_size.data(), coeff.data(), nullptr, nullptr);
    }

    /**
     * Equation (5)
     * $x_{ij} \leq (N-1) y_{ij} \forall i, j \in A, j \neq 0$
     * =>
     *    $$x_{ij} - (N-1) y_{ij} \leq 0 \forall i, j \in A, j \neq 0$
     */
    for (int i = 0; i < this->N; ++i) {
        for (int j = 1; j < this->N; ++j) {
            if (i != j) {
                int left_side[2]{x_variable_mat.at(i, j), this->y_variable_mat->at(i, j)};
                double coeff[2]{1, static_cast<double>(-this->N + 1)};

                // right side: `<= 0`
                char sense = 'L';
                const double right_side = 0.0;

                int matbeg = 0;
                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, 2, &right_side, &sense, &matbeg,
                                 left_side, coeff, nullptr, nullptr);
            }
        }
    }

    const char* filename = "cplex_model.lp\0";
    CHECKED_CPX_CALL(CPXwriteprob, env, lp, filename, nullptr);
}

template <typename T>
inline void CPLEXModel<T>::setup_parallelism() noexcept(false) {
    auto n_threads = std::thread::hardware_concurrency();
    CHECKED_CPX_CALL(CPXsetintparam, env, CPXPARAM_Threads, n_threads);
    CHECKED_CPX_CALL(CPXsetintparam, env, CPXPARAM_Parallel, CPX_PARALLEL_OPPORTUNISTIC);
}

// Set the maximum allotted time for computation to the CPLEX environment
template <typename T>
inline void CPLEXModel<T>::force_time_limit(const std::chrono::milliseconds& timeout_ms) noexcept(
    false) {
    using namespace std::chrono;
    double timeout_s = std::chrono::duration<double>(timeout_ms).count();

    // See
    // https://www.ibm.com/support/knowledgecenter/SSSA5P_12.7.1/ilog.odms.cplex.help/CPLEX/Parameters/topics/TiLim.html
    CHECKED_CPX_CALL(CPXsetdblparam, env, CPXPARAM_TimeLimit, timeout_s);
}

// Add a new column (variable) to the CPLEX environment.
// variable: letter of the variable being added.
// type: type of the variable. 'C' for continuous variables, 'B' for {0, 1} variables.
// lower_bound: lower bound of the variable.
// upper_bound: upper bound of the variable.
// ij: indexes of the variable.
// objective_coefficient: objective function coefficient of the new variable.
template <typename T>
inline void CPLEXModel<T>::add_column(char variable, char type, double lower_bound,
                                      double upper_bound, std::pair<size_t, size_t> ij,
                                      T objective_coefficient) noexcept(false) {
    auto&& [i, j] = ij;

    // Compute the name of the new variable to add to the CPLEX environment
    std::stringstream name_stream;
    name_stream << variable << '_' << i << '_' << j;
    std::string variable_name = name_stream.str();
    char* raw_variable_name = const_cast<char*>(variable_name.c_str());

    const short n_variables_to_add = 1;

    // Add the new column
    CHECKED_CPX_CALL(CPXnewcols, env, lp, n_variables_to_add, &objective_coefficient, &lower_bound,
                     &upper_bound, &type, &raw_variable_name);
}

template <typename T>
inline void CPLEXModel<T>::solve() {
    try {
        CHECKED_CPX_CALL(CPXmipopt, env, lp);
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return;
    }
}

template <typename T>
inline std::optional<PermutationPath<T>> CPLEXModel<T>::get_solution() const {
    try {
        // Access the solution objective value.
        double objective_value;

        // double* objval_p: A pointer to a variable where the objective value is stored.
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objective_value);

        // Store the values of primal variables of the selected linear problem in variable_values.
        std::vector<double> variable_values(this->n_variables);

        // CPXENVptr env: The pointer to the CPLEX environment as returned by one of the
        // CPXopenCPLEX routines. CPXLPptr lp: A pointer to a CPLEX LP problem object as returned by
        // CPXcreateprob() double *x: An array to receive the values of the primal variables for the
        // problem. If successful, x[0] through x[end-begin] will contain the solution values.
        // int begin: An integer indicating the beginning of the range of variable values to be
        // returned. int end: An integer indicating the end of the range of variable values to be
        // returned.
        CHECKED_CPX_CALL(CPXgetx, env, lp, variable_values.data(), 0, this->n_variables - 1);

        // Compute permutation path with preset distance
        PermutationPath<T> permutation_path(this->retrieve_path(std::move(variable_values)),
                                            this->distance_matrix);
        permutation_path.reset_cost(objective_value);

        return {permutation_path};
    } catch (std::exception& e) {
        // No solution exists. We return an empty optional
        return {};
    }
}

template <typename T>
inline std::vector<size_t> CPLEXModel<T>::retrieve_path(
    std::vector<double>&& variable_values) const noexcept {

    std::vector<size_t> path;
    path.reserve(this->n_variables);

    constexpr size_t depot = 0;

    // The double-ended queue is used to efficiently perform a Breadth-First Search of the
    // y variables to retrieve the solution, which is stored in path.
    std::deque<size_t> deque;
    deque.push_back(depot);

    while (!deque.empty()) {
        size_t c = deque.front();
        deque.pop_front();

        for (size_t j = 0; j < this->y_variable_mat->get_cols(); j++) {
            if (c != j && variable_values[this->y_variable_mat->at(c, j)] == 1.0) {
                path.emplace_back(c);

                if (j != depot) {
                    deque.push_back(j);
                }

                break;
            }
        }
    }

    return path;
}