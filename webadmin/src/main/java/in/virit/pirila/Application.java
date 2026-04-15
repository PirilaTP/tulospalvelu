package in.virit.pirila;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ComponentScan;

/**
 * The entry point of the Spring Boot application.
 */
@SpringBootApplication
@ComponentScan(basePackages = "in.virit.pirila")
public class Application {

    public static void main(String[] args) {
        SpringApplication.run(Application.class, args);
    }

}
