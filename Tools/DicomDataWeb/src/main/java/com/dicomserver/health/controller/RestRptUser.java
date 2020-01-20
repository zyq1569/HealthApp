package com.dicomserver.health.controller;


import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping(value = "/Health")
public class RestRptUser {
    @PostMapping(value = "/login/checkuser")
    public String loginUser(@RequestBody String dt) {
        return "H_ketty";
    }
    @RequestMapping(value = "/test")
    public String getTest() {
        return "H_test";
    }
    @PostMapping(value = "/testString")
    public String getTestString(@RequestBody String dt) {
        return "H_testString";
    }
}
