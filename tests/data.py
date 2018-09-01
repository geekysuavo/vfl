
import unittest
import vfl

# unit tests for vfl.Data
class TestData(unittest.TestCase):
  def test_defaults(self):
    # Data should initialize its properties to sane values.
    dat = vfl.Data()
    self.assertEqual(dat.N, 0)
    self.assertEqual(dat.D, 0)

  # FIXME -- write more tests

# when run as a script, run the unit tests.
if __name__ == '__main__':
  unittest.main()

