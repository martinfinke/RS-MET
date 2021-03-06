template<class T>
rsMatrix<T>::rsMatrix(size_t numRows, size_t numColumns)
{
  setSize(numRows, numColumns);
}

template<class T>
void rsMatrix<T>::setSize(size_t numRows, size_t numColumns)
{
  this->N = numRows;
  this->M = numColumns;
  data.resize(this->N * this->M);
  this->d = &data[0];
}

// rsFillWithValue
