import pathlib
import unittest

import kielet


class DfmTextTests(unittest.TestCase):
    def test_caption_starting_on_next_line(self):
        value, end = kielet.dfm_arvo("\n  'More class information'\n  Width = 448\n", 0)
        self.assertEqual(value, ['More class information'])

    def test_multiline_caption_preserves_characters_and_spaces(self):
        source = "\n  'Lis'#228#228' tietoa ' +\n  'sarjasta'\n  Width = 448\n"
        value, end = kielet.dfm_arvo(source, 0)
        self.assertEqual(value, ['Lisää tietoa sarjasta'])
        self.assertEqual(source[end:], '  Width = 448\n')

    def test_string_list_keeps_separate_items(self):
        source = "\n  (\n    'Ensimm'#228'inen'\n    'Toinen ' +\n    'rivi'\n    '')\n"
        value, end = kielet.dfm_arvo(source, 0)
        self.assertEqual(value, ['Ensimmäinen', 'Toinen rivi', ''])

    def test_inline_empty_string_and_quoted_apostrophe(self):
        for source, expected in [("''\n", ''), ("'It''s ready'\n", "It's ready")]:
            with self.subTest(source=source):
                value, end = kielet.dfm_arvo(source, 0)
                self.assertEqual(value, [expected])
                self.assertEqual(end, len(source))

    def test_non_string_does_not_consume_next_property(self):
        source = "\n  448\n  Caption = 'Class data'\n"
        value, end = kielet.dfm_arvo(source, 0)
        self.assertIsNone(value)
        self.assertEqual(source[end:], "  Caption = 'Class data'\n")

    def test_class_data_form_help_is_extracted(self):
        form = pathlib.Path(kielet.V52) / 'cbHk' / 'UnitSarjatiedot.dfm'
        labels = [text for obj, cls, prop, text, line in kielet.dfm_tekstit(form)
                  if obj.endswith('.Label3') and prop == 'Caption']
        self.assertEqual(labels, [
            'Lisää tietoa sarjasta on kaavakkeella, joka avautuu '
            'kaksoisklikkaamalla sarjan riviä taulukossa'
        ])


if __name__ == '__main__':
    unittest.main()
