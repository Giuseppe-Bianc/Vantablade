/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "glm_matld.hpp"

#define PRETTY_PRINT
#define GLMP_FUN_QUAL GLM_FUNC_QUALIFIER

namespace glmp {
    namespace detail {

        /**
         * @brief String label for the boolean value 'true'.
         */
        static inline constexpr const char *LabelTrue = "true";

        /**
         * @brief String label for the boolean value 'false'.
         */
        static inline constexpr const char *LabelFalse = "false";

        template <typename T> [[nodiscard]] consteval const char *typePrefix() noexcept {
            if constexpr(std::is_same_v<T, double>)
                return "d";
            else if constexpr(std::is_same_v<T, long double>)
                return "ld";
            else if constexpr(std::is_same_v<T, bool>)
                return "b";
            else if constexpr(std::is_same_v<T, uint8_t>)
                return "u8";
            else if constexpr(std::is_same_v<T, int8_t>)
                return "i8";
            else
                return "";
        }

        [[nodiscard]] static constexpr const char *boolLabel(bool v) noexcept { return v ? LabelTrue : LabelFalse; }

        /**
         * @brief Template structure for converting glm vector types to strings.
         * @tparam matType The glm vector type.
         */
        template <typename matType> struct compute_to_string {};

        /**
         * @brief Specialization for glm::vec<1, bool, Q>.
         */
        template <glm::qualifier Q> struct compute_to_string<glm::vec<1, bool, Q>> {
            /**
             * @brief Converts glm::vec<1, bool, Q> to a string.
             * @param x The input vector.
             * @return String representation of the vector.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<1, bool, Q> const &x) {
                return FORMAT("bvec1({})", boolLabel(x[0]));
            }
        };
        /**
         * @brief Specialization for glm::vec<2, bool, Q>.
         */
        template <glm::qualifier Q> struct compute_to_string<glm::vec<2, bool, Q>> {
            /**
             * @brief Converts glm::vec<2, bool, Q> to a string.
             * @param x The input vector.
             * @return String representation of the vector.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<2, bool, Q> const &x) {
                return FORMAT("bvec2({}, {})", boolLabel(x[0]), boolLabel(x[1]));
            }
        };
        /**
         * @brief Specialization for glm::vec<3, bool, Q>.
         */
        template <glm::qualifier Q> struct compute_to_string<glm::vec<3, bool, Q>> {
            /**
             * @brief Converts glm::vec<3, bool, Q> to a string.
             * @param x The input vector.
             * @return String representation of the vector.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<3, bool, Q> const &x) {
                return FORMAT("bvec3({}, {}, {})", boolLabel(x[0]), boolLabel(x[1]), boolLabel(x[2]));
            }
        };

        /**
         * @brief Specialization for glm::vec<4, bool, Q>.
         */
        template <glm::qualifier Q> struct compute_to_string<glm::vec<4, bool, Q>> {
            /**
             * @brief Converts glm::vec<4, bool, Q> to a string.
             * @param x The input vector.
             * @return String representation of the vector.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<4, bool, Q> const &x) {
                return FORMAT("bvec4({}, {}, {}, {})", boolLabel(x[0]), boolLabel(x[1]), boolLabel(x[2]), boolLabel(x[3]));
            }
        };

        // ── generic scalar vectors ────────────────────────────────────────────────
        /**
         * @brief Provides functions to convert glm::vec types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::vec<1, T, Q>> {
            /**
             * @brief Convert glm::vec1 to string.
             * @param x The glm::vec1 to be converted.
             * @return The string representation of glm::vec1.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<1, T, Q> const &x) {
                return FORMAT("{}vec1({})", typePrefix<T>(), x[0]);
            }
        };

        /**
         * @brief Provides functions to convert glm::vec types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::vec<2, T, Q>> {
            /**
             * @brief Convert glm::vec2 to string.
             * @param x The glm::vec2 to be converted.
             * @return The string representation of glm::vec2.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<2, T, Q> const &x) {
                return FORMAT("{}vec2({}, {})", typePrefix<T>(), x[0], x[1]);
            }
        };

        /**
         * @brief Provides functions to convert glm::vec types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::vec<3, T, Q>> {
            /**
             * @brief Convert glm::vec3 to string.
             * @param x The glm::vec3 to be converted.
             * @return The string representation of glm::vec3.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<3, T, Q> const &x) {
                return FORMAT("{}vec3({}, {}, {})", typePrefix<T>(), x[0], x[1], x[2]);
            }
        };

        /**
         * @brief Provides functions to convert glm::vec types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::vec<4, T, Q>> {
            /**
             * @brief Convert glm::vec4 to string.
             * @param x The glm::vec4 to be converted.
             * @return The string representation of glm::vec4.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::vec<4, T, Q> const &x) {
                return FORMAT("{}vec4({}, {}, {}, {})", typePrefix<T>(), x[0], x[1], x[2], x[3]);
            }
        };

        // ── matrices ─────────────────────────────────────────────────────────────
        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<2, 2, T, Q>> {
            /**
             * @brief Convert glm::mat2x2 to string.
             * @param x The  glm::mat2x2 to be converted.
             * @return The string representation of  glm::mat2x2.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<2, 2, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat2x2(\n ({},{}),\n ({},{}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0], x[1][1]);
#else
                return FORMAT("{}mat2x2(({},{}), ({},{}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0], x[1][1]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<2, 3, T, Q>> {
            /**
             * @brief Convert glm::mat2x3 to string.
             * @param x The  glm::mat2x3 to be converted.
             * @return The string representation of  glm::mat2x3.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<2, 3, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat2x3(\n ({}, {}, {}),\n ({}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2], x[1][0], x[1][1],
                              x[1][2]);
#else
                return FORMAT("{}mat2x3(({}, {}, {}), ({}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2], x[1][0], x[1][1],
                              x[1][2]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<2, 4, T, Q>> {
            /**
             * @brief Convert glm::mat2x4 to string.
             * @param x The  glm::mat2x4 to be converted.
             * @return The string representation of  glm::mat2x4.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<2, 4, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat2x4(({}, {}, {}, {}),\n ({}, {}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2], x[0][3],
                              x[1][0], x[1][1], x[1][2], x[1][3]);
#else
                return FORMAT("{}mat2x4(({}, {}, {}, {}), ({}, {}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2], x[0][3], x[1][0],
                              x[1][1], x[1][2], x[1][3]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<3, 2, T, Q>> {
            /**
             * @brief Convert glm::mat3x2 to string.
             * @param x The  glm::mat3x2 to be converted.
             * @return The string representation of  glm::ma3x2.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<3, 2, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat3x2(\n ({}, {}),\n ({}, {}),\n ({}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0], x[1][1], x[2][0],
                              x[2][1]);
#else
                return FORMAT("{}mat3x2(({}, {}), ({}, {}), ({}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0], x[1][1], x[2][0],
                              x[2][1]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<3, 3, T, Q>> {
            /**
             * @brief Convert glm::mat3x3 to string.
             * @param x The  glm::mat3x3 to be converted.
             * @return The string representation of  glm::mat3x3.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<3, 3, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat3x3(\n ({}, {}, {}),\n ({}, {}, {}),\n ({}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2],
                              x[1][0], x[1][1], x[1][2], x[2][0], x[2][1], x[2][2]);
#else
                return FORMAT("{}mat3x3(({}, {}, {}), ({}, {}, {}), ({}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2], x[1][0],
                              x[1][1], x[1][2], x[2][0], x[2][1], x[2][2]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<3, 4, T, Q>> {
            /**
             * @brief Convert glm::mat3x4 to string.
             * @param x The glm::mat3x4 to be converted.
             * @return The string representation of glm::mat3x4.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<3, 4, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat3x4(\n ({}, {}, {}, {}),\n ({}, {}, {}, {}),\n ({}, {}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1],
                              x[0][2], x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2], x[2][3]);
#else
                return FORMAT("{}mat3x4(({}, {}, {}, {}), ({}, {}, {}, {}), ({}, {}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[0][2],
                              x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2], x[2][3]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<4, 2, T, Q>> {
            /**
             * @brief Convert glm::mat4x2 to string.
             * @param x The  glm::mat4x2 to be converted.
             * @return The string representation of  glm::mat4x2.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<4, 2, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat4x2(\n ({}, {}),\n ({}, {}),\n ({}, {}),\n ({}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0],
                              x[1][1], x[2][0], x[2][1], x[3][0], x[3][1]);
#else
                return FORMAT("{}mat4x2(({}, {}), ({}, {}), ({}, {}), ({}, {}))", typePrefix<T>(), x[0][0], x[0][1], x[1][0], x[1][1],
                              x[2][0], x[2][1], x[3][0], x[3][1]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<4, 3, T, Q>> {
            /**
             * @brief Convert glm::mat4x2 to string.
             * @param x The  glm::mat4x2 to be converted.
             * @return The string representation of  glm::mat4x2.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<4, 3, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat4x3(\n ({}, {}, {}),\n ({}, {}, {}),\n ({}, {}, {}),\n ({}, {}, {}))", typePrefix<T>(), x[0][0],
                              x[0][1], x[0][2], x[1][0], x[1][1], x[1][2], x[2][0], x[2][1], x[2][2], x[3][0], x[3][1], x[3][2]);
#else
                return FORMAT("{}mat4x3(({}, {}, {}), ({}, {}, {}), ({}, {}, {}), ({}, {}, {}))", typePrefix<T>(), x[0][0], x[0][1],
                              x[0][2], x[1][0], x[1][1], x[1][2], x[2][0], x[2][1], x[2][2], x[3][0], x[3][1], x[3][2]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::mat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::mat<4, 4, T, Q>> {
            /**
             * @brief Convert glm::mat4x4 to string.
             * @param x The glm::mat4x4 to be converted.
             * @return The string representation of  glm::mat4x4.
             */
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::mat<4, 4, T, Q> const &x) {
#ifdef PRETTY_PRINT
                return FORMAT("{}mat4x4(\n ({}, {}, {}, {}),\n ({}, {}, {}, {}),\n ({}, {}, {}, {}),\n ({}, {}, {}, {}))", typePrefix<T>(),
                              x[0][0], x[0][1], x[0][2], x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2], x[2][3],
                              x[3][0], x[3][1], x[3][2], x[3][3]);
#else
                return FORMAT("{}mat4x4(({}, {}, {}, {}), ({}, {}, {}, {}), ({}, {}, {}, {}), ({}, {}, {}, {}))", typePrefix<T>(), x[0][0],
                              x[0][1], x[0][2], x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2], x[2][3], x[3][0],
                              x[3][1], x[3][2], x[3][3]);
#endif
            }
        };

        /**
         * @brief Provides functions to convert glm::qua types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::qua<T, Q>> {
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::qua<T, Q> const &q) {  // NOLINT(*-identifier-length)
                return FORMAT("{}quat({}, [{}, {}, {}])", typePrefix<T>(), q.w, q.x, q.y, q.z);
            }
        };

        /**
         * @brief Provides functions to convert glm::tdualquat types to strings.
         */
        template <typename T, glm::qualifier Q> struct compute_to_string<glm::tdualquat<T, Q>> {
            [[nodiscard]] GLMP_FUN_QUAL static std::string call(glm::tdualquat<T, Q> const &x) {
                return FORMAT("{}dualquat(({}, [{}, {}, {}]), ({}, [{}, {}, {}]))", typePrefix<T>(), x.real.w, x.real.x, x.real.y, x.real.z,
                              x.dual.w, x.dual.x, x.dual.y, x.dual.z);
            }
        };

    }  // namespace detail

    template <typename T>
    concept GlmFormattable = requires(const T &v) {
        { detail::compute_to_string<T>::call(v) } -> std::same_as<std::string>;
    };

    /**
     * @brief Converts a generic object into a string.
     * This function uses the compute_to_string class to convert the input object into a string.
     * The compute_to_string class must be specialized for the matType type.
     * @tparam matType The type of the object to be converted into a string.
     * @param x The object to be converted into a string.
     * @return A string that represents the object x.
     * @note This function is marked as [[nodiscard]] to indicate that the compiler should emit a warning if the return value is
     * not used.
     */
    template <GlmFormattable matType> [[nodiscard]] GLMP_FUN_QUAL std::string to_string(matType const &x) {
        return detail::compute_to_string<matType>::call(x);
    }

}  // namespace glmp

// NOLINTEND(*-include-cleaner)