# -*- coding: utf-8 -*-

################################################################
# zopyx.txng3.ext
# (C) 2005-2016, Andreas Jung, ZOPYX, www.zopyx.com
################################################################


import sys
import os
import unittest
from zopyx.txng3.ext.normalizer import Normalizer

__basedir__ = os.getcwd()


class TestNormalizer(unittest.TestCase):

    def _doTest(self, text, table):

        N = Normalizer(table)
        got = N.normalize(text)

        expected = text
        for old, new in table:
            expected = expected.replace(old, new)

        self.assertEqual(got, expected)

    def testSimple(self):

        N = Normalizer([])
        self.assertEqual(N.getTable(), [])

        N = Normalizer([('a', 'b'), ('c', 'd'), ('a', 'b')])
        self.assertEqual(N.getTable(), [('a', 'b'), ('c', 'd'), ('a', 'b')])

    def test1(self):

        table = [('a', 'b'), ('c', 'd')]
        text = 'the quick brown fox jumps over the lazy dog'
        self._doTest(text, table)

    def test2(self):

        table = [('a', 'bb'), ('bb', 'cc')]
        text = 'the quick brown fox jumps over the lazy dog'
        self._doTest(text, table)

    def test3(self):

        table = [('foo', 'bar')]
        text = 'the quick brown fox jumps over the lazy dog'
        self._doTest(text, table)

    def test4(self):

        table = [('ä', 'ae'), ('ö', 'oe')]
        text = 'Bei den dreitägigen Angriffen seien auch bis'\
               'auf einen alle Flugplätze der Taliban zerstört worden'
        self._doTest(text, table)


def test_suite():
    s = unittest.TestSuite()
    s.addTest(unittest.makeSuite(TestNormalizer))
    return s


def main():
    unittest.TextTestRunner().run(test_suite())


def debug():
    test_suite().debug()


def pdebug():
    import pdb
    pdb.run('debug()')

if __name__ == '__main__':
    if len(sys.argv) > 1:
        globals()[sys.argv[1]]()
    else:
        main()
