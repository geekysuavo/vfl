
import unittest
import vfl

# unit tests for vfl.Datum
class TestDatum(unittest.TestCase):
  def test_defaults(self):
    # Datum should initialize its properties to sane values.
    dat = vfl.Datum()
    self.assertEqual(dat.dims, 0)
    self.assertEqual(dat.output, 0)
    self.assertEqual(dat.y, 0.0)
    self.assertEqual(dat.x, [])

  def test_dims(self):
    # dimensionalities track the input location size.
    dat = vfl.Datum(x = [3])
    self.assertEqual(dat.dims, 1)

    # dimensionalities are immutable.
    with self.assertRaises(AttributeError):
      dat.dims = 10

  def test_output(self):
    # output indices can be set at creation.
    dat = vfl.Datum(output = 2)
    self.assertEqual(dat.output, 2)

    # output indices are mutable.
    dat.output = 3
    self.assertEqual(dat.output, 3)

    # negative output indices are not allowed.
    with self.assertRaises(OverflowError):
      dat.output = -1

    # negative output indices are not allowed.
    with self.assertRaises(OverflowError):
      dat = vfl.Datum(output = -2)

    # non-integer output indices are not allowed.
    with self.assertRaises(TypeError):
      dat.output = 'foo'

  def test_input(self):
    # input locations can be set at creation.
    dat = vfl.Datum(x = (1, 2, 3))
    self.assertEqual(dat.x, [1, 2, 3])

    # input locations are mutable.
    dat.x = [4, 5]
    self.assertEqual(len(dat), 2)
    self.assertEqual(dat.dims, 2)
    self.assertEqual(dat.x, [4, 5])

    # non-float-sequence input locations are not allowed.
    with self.assertRaises(TypeError):
      dat.x = 'foo'

  def test_value(self):
    # values can be set at creation.
    dat = vfl.Datum(y = 3.125)
    self.assertEqual(dat.y, 3.125)

    # values are mutable.
    dat.y = -6.25
    self.assertEqual(dat.y, -6.25)

    # non-float values are not allowed.
    with self.assertRaises(TypeError):
      dat.y = 'foo'

    # non-float values are not allowed.
    with self.assertRaises(TypeError):
      dat = vfl.Datum(y = 'bar')

  def test_empty_sequence(self):
    # Datum sequences default to empty (as does 'x')
    dat = vfl.Datum()
    self.assertEqual(len(dat), 0)
    with self.assertRaises(IndexError):
      z = dat[0]

  def test_sequence(self):
    # Datum is a sequence.
    dat = vfl.Datum(x = [1, -2, 3, -4])
    self.assertEqual(len(dat), 4)
    self.assertEqual(list(dat), [1, -2, 3, -4])

    # sequence elements are mutable.
    dat[1] = -222
    dat[2] = 333
    self.assertEqual(list(dat), [1, -222, 333, -4])

    # sequence elements must be floats.
    with self.assertRaises(TypeError):
      dat[3] = 'baz'

# when run as a script, run the unit tests.
if __name__ == '__main__':
  unittest.main()

