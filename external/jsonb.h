#ifndef PJSONB_H
#define PJSONB_H

enum pjsonb_key_state {
  PJSONB_NONE,
  PJSONB_TO_CLOSE
};

struct pjsonb {
  char *string;
  int position;
  enum pjsonb_key_state key_state;
};

void pjsonb_init(struct pjsonb *builder);

void pjsonb_end(struct pjsonb *builder);

void pjsonb_free(struct pjsonb *builder);

void pjsonb_set_int(struct pjsonb *builder, const char *key, int value);

void pjsonb_set_float(struct pjsonb *builder, const char *key, float value);

void pjsonb_set_bool(struct pjsonb *builder, const char *key, int value);

void pjsonb_set_string(struct pjsonb *builder, const char *key, const char *value);

void pjsonb_enter_object(struct pjsonb *builder, const char *key);

void pjsonb_leave_object(struct pjsonb *builder);

void pjsonb_enter_array(struct pjsonb *builder, const char *key);

void pjsonb_leave_array(struct pjsonb *builder);

#endif