package in.virit.pirila.data;

import jakarta.validation.Constraint;
import jakarta.validation.Payload;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
@Constraint(validatedBy = CardNotInUseValidator.class)
public @interface CardNotInUse {
    String message() default "Kortti on jo käytössä";
    Class<?>[] groups() default {};
    Class<? extends Payload>[] payload() default {};
}
