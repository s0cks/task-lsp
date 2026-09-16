package lsp

import (
	"log"

	"taskfile-lsp/internal/document"
)

type Document struct {
	URI         string
	LanguageID  string
	Version     int
	Text        string
	Diagnostics []Diagnostic

	parsed *document.Document
}

// Access hands fn the current parse. The server is single-threaded, so
// there's no locking here -- but the rule stays the same as it would be
// under concurrency: fn must not let doc, or any Task/Var/Ref/Node derived
// from it, escape the closure. Extract plain values (strings, Pos, Range)
// instead. That discipline is what keeps a later reparse's Free() from
// invalidating something a handler is still holding onto.
func (d *Document) Access(fn func(doc *document.Document, text string)) {
	fn(d.parsed, d.Text)
}

func (d *Document) replace(version int, text string, parsed *document.Document, diags []Diagnostic) {
	old := d.parsed
	d.Version = version
	d.Text = text
	d.parsed = parsed
	d.Diagnostics = diags

	if old != nil {
		old.Free()
	}
}

func (d *Document) close() {
	old := d.parsed
	d.parsed = nil

	if old != nil {
		old.Free()
	}
}

type DocumentStore struct {
	log  *log.Logger
	docs map[string]*Document
}

func NewDocumentStore(logger *log.Logger) *DocumentStore {
	return &DocumentStore{log: logger, docs: make(map[string]*Document)}
}

func (s *DocumentStore) Open(doc *Document) []Diagnostic {
	parsed, diags := parseAndConvert(doc.Text)
	doc.parsed = parsed
	doc.Diagnostics = diags
	s.docs[doc.URI] = doc
	return diags
}

func (s *DocumentStore) Get(uri string) (*Document, bool) {
	d, ok := s.docs[uri]
	return d, ok
}

func (s *DocumentStore) Update(uri string, version int, text string) []Diagnostic {
	d, ok := s.docs[uri]
	if !ok {
		return nil
	}

	parsed, diags := parseAndConvert(text)
	d.replace(version, text, parsed, diags)
	return diags
}

func (s *DocumentStore) Close(uri string) {
	d, ok := s.docs[uri]
	if !ok {
		return
	}
	delete(s.docs, uri)
	d.close()
}
