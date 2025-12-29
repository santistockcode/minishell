// Test different input and errors on readline wrapper 

// Here we make use of indirection via syswrap (not just injection via env variables)


// Brainstorming:
// Qué pasa con un here doc en el que solo se pasa el delimiter?
// Debo comerme el \n final?
// Qué pasa con las líneas en blanco? las incluyo o me las salto?
// Heredoc buffering: prefer fetch_here_doc_from_user(...) returning the full content; then set_here_doc(...) writes that to a temp file and replaces the command’s heredoc redir with R_IN pointing to that temp path. 