package main

import (
    "net/http"
    "io/ioutil"
    "html/template"
    "regexp"
)

type Page struct {
    Title string
    Body []byte // NOTE: []byte for io
}

var dataDir = "data/"
var tmplDir = "tmpl/"

// Return nil if all goes well
func (p *Page) save() error {
    filename := dataDir + p.Title + ".txt"
    return ioutil.WriteFile(filename, p.Body, 0600) // read-write permissions - 0600
}

func loadPage(title string) (*Page, error) {
    filename := dataDir + title + ".txt"
    body, err := ioutil.ReadFile(filename)
    if err != nil {
        return nil, err
    }
    return &Page{Title: title, Body: body}, nil
}

func viewHandler(w http.ResponseWriter, r *http.Request, title string) {
    p, err := loadPage(title)
    if err != nil {
        http.Redirect(w, r, "/edit/"+title, http.StatusFound) // http.StatusFound(302)
        return
    }
    renderTemplate(w, "view", p)
}

func editHandler(w http.ResponseWriter, r *http.Request, title string) {
    p, err := loadPage(title)
    if err != nil {
        p = &Page{Title: title}
    }
    renderTemplate(w, "edit", p)
}

func saveHandler(w http.ResponseWriter, r *http.Request, title string) {
    body := r.FormValue("body") // Return value is string type
    p := &Page{Title: title, Body: []byte(body)}
    err := p.save()
    if err != nil {
        http.Error(w, err.Error(), http.StatusInternalServerError)
        return
    }
    http.Redirect(w, r, "/view/"+title, http.StatusFound)
}

var templates = template.Must(template.ParseFiles("edit.html", "view.html"))
var pageNameReplacer = regexp.MustCompile("\\[([a-zA-Z0-9]+)\\]")

func renderTemplate(w http.ResponseWriter, tmpl string, p *Page) {
    fp := &Page{Title: p.Title}
    if tmpl == "view" {
        fp.Body = pageNameReplacer.ReplaceAll(p.Body, []byte("HTML(`<a href=\"/view/$1\">$1</a>`)"))
    } else {
        fp.Body = p.Body
    }
    err := templates.ExecuteTemplate(w, tmpl + ".html", fp)
    if err != nil {
        http.Error(w, err.Error(), http.StatusInternalServerError)
        return
    }
}

const lenPath = len("/view/")

var titleValidator = regexp.MustCompile("^[a-zA-Z0-9]+$") // exit app if failed

func makeHandler(fn func (http.ResponseWriter, *http.Request, string)) http.HandlerFunc {
    return func(w http.ResponseWriter, r *http.Request) {
        // Here we will extract the page title from the Request,
		// and call the provided handler 'fn'
        title := r.URL.Path[lenPath:]
        if !titleValidator.MatchString(title) {
            http.NotFound(w, r)
            return
            //err = errors.New("Invalid Page Title")
        }
        fn(w, r, title)
    }
}

func rootHandler(w http.ResponseWriter, r *http.Request) {
    http.Redirect(w, r, "/view/FrontPage", http.StatusFound) // http.StatusFound(302)
}

func main() {
    http.HandleFunc("/view/", makeHandler(viewHandler))
    http.HandleFunc("/edit/", makeHandler(editHandler))
    http.HandleFunc("/save/", makeHandler(saveHandler))
    http.HandleFunc("/", rootHandler)
    http.ListenAndServe(":8080", nil)
}
