#include <cstddef>
#include <vector>
template <typename T>
struct Matrix {
    Matrix(size_t rRows, size_t rCols) : // NOLINT
        mRows(rRows), mCols(rCols), mContainer(mRows * mCols) {}
    auto operator()(size_t i, size_t j) -> T& { return mContainer[i + mRows * j]; }

  private:
    size_t mRows;
    size_t mCols;
    std::vector<T> mContainer{};
};
