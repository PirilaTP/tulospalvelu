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


    def test_read_only_edit_text_is_extracted(self):
        form = pathlib.Path(kielet.V52) / 'cbHk' / 'UnitEmit.dfm'
        texts = [text for obj, cls, prop, text, line in kielet.dfm_tekstit(form)
                 if prop == 'Text']
        self.assertIn('Käsittelyyn kortti', texts)
        # muokattavan kentän tekstiä ('0') ei poimita käännettäväksi kuin vain luku -kentistä
        self.assertNotIn('EdtSarja', [t for obj, cls, prop, t, line in kielet.dfm_tekstit(
            pathlib.Path(kielet.V52) / 'cbHk' / 'UnitVaToisto.dfm')])


class UntranslatedTextTests(unittest.TestCase):
    def found(self, code):
        return [(kind, text) for line, kind, text in kielet.kaantamattomat_teksti(code)]

    def test_property_assignment_without_tr_is_reported(self):
        code = 'void f() {\n  EdtMsg->Text = L"Lue kortti tai syötä numero";\n}\n'
        self.assertEqual(self.found(code), [('sijoitus', 'Lue kortti tai syötä numero')])

    def test_wrapped_text_is_not_reported(self):
        code = 'void f() {\n  EdtMsg->Text = TR(L"Lue kortti");\n  header = TR(L"KORJAA    ");\n}\n'
        self.assertEqual(self.found(code), [])

    def test_joined_text_is_reported_as_concatenation(self):
        code = 'void f() {\n  SG->Cells[0][1] = UnicodeString(i+1) + L". vaihe";\n}\n'
        self.assertEqual(self.found(code), [('yhdistelma', '. vaihe')])

    def test_compared_text_is_not_reported(self):
        code = 'void f() {\n  if (Edt->Text == L"Kaikki sarjat") g();\n}\n'
        self.assertEqual(self.found(code), [])

    def test_message_box_without_tr_is_reported(self):
        code = 'void f() {\n  Application->MessageBoxW(L"Tiedostoa ei löydy", L"Virhe", MB_OK);\n}\n'
        self.assertIn(('ilmoitus', 'Tiedostoa ei löydy'), self.found(code))

    def test_commented_out_code_is_not_reported(self):
        code = 'void f() {\n/*\n  Edt->Text = L"Lue kortti";\n*/\n}\n'
        self.assertEqual(self.found(code), [])


try:
    import PIL  # noqa: F401
    import mitat
except ImportError:
    mitat = None


@unittest.skipIf(mitat is None, 'mittaus vaatii Pillow-kirjaston')
class WrapMeasureTests(unittest.TestCase):
    def setUp(self):
        self.m = mitat.Mittari()
        self.f = self.m.fontti('Arial', 13, False)

    def test_words_wrap_at_spaces(self):
        self.assertEqual(self.m.rivit('Ta bort valda', self.f, 41), 2)

    def test_word_wider_than_control_does_not_fit(self):
        # Windows katkaisisi sanan kesken (Byt tävlande, ändra stämplingsuppgif-ter)
        self.assertIsNone(self.m.rivit('stämplingsuppgifter', self.f, 91))

    def test_hotkey_marker_is_not_measured(self):
        self.assertEqual(self.m.rivit('&Lisää', self.f, 200), 1)


if __name__ == '__main__':
    unittest.main()
