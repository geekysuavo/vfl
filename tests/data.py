
import unittest, os
import vfl

# unit tests for vfl.Data
class TestData(unittest.TestCase):
  def test_defaults(self):
    # Data should initialize its properties to sane values.
    dat = vfl.Data()
    self.assertEqual(len(dat), 0)
    self.assertEqual(dat.dims, 0)

  def test_len(self):
    # counts should track the number of observations.
    dat = vfl.Data(grid = [[2, 1, 5]])
    self.assertEqual(len(dat), 4)

  def test_dims(self):
    # dimensionalities should track the contained data.
    dat = vfl.Data(grid = [[2, 1, 5]])
    self.assertEqual(dat.dims, 1)

    # dimensionalities are immutable.
    with self.assertRaises(AttributeError):
      dat.dims = 2

  def test_write(self):
    # build a dataset.
    datA = vfl.Data(grid = [[1, 1, 2], [1, 1, 3], [1, 1, 5]],
                    outputs = [0, 1])

    # write the dataset to a file and read that file back.
    filename = 'data.py.tmp'
    datA.write(file = filename)
    datB = vfl.Data(file = filename)
    os.remove(filename)

    # check that the data are identical.
    self.assertEqual(len(datA), len(datB))
    self.assertEqual(datA.dims, datB.dims)
    for i in range(len(datA)):
      self.assertEqual(datA[i].output, datB[i].output)
      self.assertEqual(datA[i].x, datB[i].x)
      self.assertEqual(datA[i].y, datB[i].y)

  def test_augment_from_file(self):
    # write a few temporary files.
    files = ['data.py.tmp.1', 'data.py.tmp.2', 'data.py.tmp.3']
    with open(files[0], 'w') as f:
      f.write('\n'.join(['# 3 1',
                         '0 1.5 -0.1',
                         '0 2.5 -0.01',
                         '0 3.5 -0.001']))
    with open(files[1], 'w') as f:
      f.write('\n'.join(['# 2 1',
                         '0 1.25 1',
                         '1 2.25 0.1']))
    with open(files[2], 'w') as f:
      f.write('\n'.join(['# 2 2',
                         '0 1.25 5.5 1',
                         '0 2.25 7.3 0']))

    # Data accept files at creation.
    dat = vfl.Data(file = files[0])
    self.assertEqual(len(dat), 3)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.output for d in dat], [0, 0, 0])
    self.assertEqual([d.x[0] for d in dat], [1.5, 2.5, 3.5])
    self.assertEqual([d.y for d in dat], [-0.1, -0.01, -0.001])

    # Data accept files through augment().
    dat.augment(file = files[1])
    self.assertEqual(len(dat), 5)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.output for d in dat], [0, 0, 0, 0, 1])
    self.assertEqual([d.x[0] for d in dat], [1.25, 1.5, 2.5, 3.5, 2.25])
    self.assertEqual([d.y for d in dat], [1, -0.1, -0.01, -0.001, 0.1])

    # dimensionalities of new files must match the Data.
    with self.assertRaises(IOError):
      dat.augment(file = files[2])

    # remove the temporary files.
    for f in files:
      os.remove(f)

  def test_augment_from_datum(self):
    # Data accept single observations at creation.
    dat = vfl.Data(datum = vfl.Datum(x = [1, 0], y = -2, output = 1))
    self.assertEqual(len(dat), 1)
    self.assertEqual(dat.dims, 2)
    self.assertEqual([d.y for d in dat], [-2])

    # Data accept single observations through augment().
    dat.augment(datum = vfl.Datum(x = [-1, 0], y = -3, output = 2))
    self.assertEqual(len(dat), 2)
    self.assertEqual(dat.dims, 2)
    self.assertEqual([d.y for d in dat], [-2, -3])

    # dimensionalities of new observations must match the Data
    with self.assertRaises(RuntimeError):
      dat.augment(datum = vfl.Datum(x = [3]))

  def test_augment_from_data(self):
    # Data accept datasets at creation.
    datA = vfl.Data(grid = [[1, 1, 3]])
    dat = vfl.Data(data = datA)
    self.assertEqual(len(dat), 3)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.output for d in dat], [0, 0, 0])
    self.assertEqual([d.x[0] for d in dat], [1, 2, 3])

    # Data accept datasets through augment().
    datB = vfl.Data(grid = [[2, 1, 4]], output = 1)
    dat.augment(data = datB)
    self.assertEqual(len(dat), 6)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.output for d in dat], [0, 0, 0, 1, 1, 1])
    self.assertEqual([d.x[0] for d in dat], [1, 2, 3, 2, 3, 4])

    # dimensionalities of new datasets must match the Data.
    with self.assertRaises(RuntimeError):
      datC = vfl.Data(grid = [[1, 1, 3], [1, 1, 3]])
      dat.augment(data = datC)

  def test_augment_from_grid(self):
    # Data accept grids at creation.
    dat = vfl.Data(grid = [[1, 2, 5]])
    self.assertEqual(len(dat), 3)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.x[0] for d in dat], [1, 3, 5])

    # Data accept grids through augment().
    dat.augment(grid = [[2, 2, 6]])
    self.assertEqual(len(dat), 6)
    self.assertEqual(dat.dims, 1)
    self.assertEqual([d.x[0] for d in dat], [1, 2, 3, 4, 5, 6])

    # grids must be matrices.
    with self.assertRaises(TypeError):
      dat = vfl.Data(grid = 'foo')

    # grids must have equal row counts.
    with self.assertRaises(TypeError):
      dat = vfl.Data(grid = [[1, 1, 3], [1, 2]])

    # dimensionalities of new grids must match the Data.
    with self.assertRaises(RuntimeError):
      dat.augment(grid = [[1, 1, 3], [1, 1, 3]])

  def test_empty_sequence(self):
    # Data sequences default to empty.
    dat = vfl.Data()
    self.assertEqual(len(dat), 0)
    with self.assertRaises(IndexError):
      z = dat[0]

  def test_sequence(self):
    # Data is a sequence.
    dat = vfl.Data(grid = [[1, 1, 3]], outputs = [0, 1])
    self.assertEqual(len(dat), 6)
    self.assertEqual([d.output for d in dat], [0, 0, 0, 1, 1, 1])
    self.assertEqual([d.x[0] for d in dat], [1, 2, 3, 1, 2, 3])

    # sequence elements are mutable, but data are re-sorted.
    dat[2] = vfl.Datum(output = 2, x = [5], y = 0)
    self.assertEqual([d.output for d in dat], [0, 0, 1, 1, 1, 2])
    self.assertEqual([d.x[0] for d in dat], [1, 2, 1, 2, 3, 5])

    # sequence elements must be Datum objects.
    with self.assertRaises(TypeError):
      dat[2] = 'baz'

# when run as a script, run the unit tests.
if __name__ == '__main__':
  unittest.main()

