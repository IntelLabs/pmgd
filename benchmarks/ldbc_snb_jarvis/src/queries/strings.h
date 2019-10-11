#pragma once

#include "pmgd.h"

struct strings_t {
    PMGD::StringID Person;
    PMGD::StringID Post;
    PMGD::StringID Tag;
    PMGD::StringID birthday;
    PMGD::StringID browserUsed;
    PMGD::StringID classYear;
    PMGD::StringID containerOf;
    PMGD::StringID content;
    PMGD::StringID country;
    PMGD::StringID creationDate;
    PMGD::StringID email;
    PMGD::StringID emailaddress;
    PMGD::StringID firstName;
    PMGD::StringID gender;
    PMGD::StringID hasCreator;
    PMGD::StringID hasInterest;
    PMGD::StringID hasMember;
    PMGD::StringID hasTag;
    PMGD::StringID hasType;
    PMGD::StringID id;
    PMGD::StringID imageFile;
    PMGD::StringID isLocatedIn;
    PMGD::StringID isPartOf;
    PMGD::StringID isSubclassOf;
    PMGD::StringID joinDate;
    PMGD::StringID knows;
    PMGD::StringID language;
    PMGD::StringID lastName;
    PMGD::StringID likes;
    PMGD::StringID locationIP;
    PMGD::StringID name;
    PMGD::StringID replyOf;
    PMGD::StringID speaks;
    PMGD::StringID studyAt;
    PMGD::StringID title;
    PMGD::StringID type;
    PMGD::StringID workAt;
    PMGD::StringID workFrom;

    void init()
    {
        if (Person == 0) {
            Person = PMGD::StringID("Person");
            Post = PMGD::StringID("Post");
            Tag = PMGD::StringID("Tag");
            birthday = PMGD::StringID("birthday");
            browserUsed = PMGD::StringID("browserUsed");
            classYear = PMGD::StringID("classYear");
            containerOf = PMGD::StringID("containerOf");
            content = PMGD::StringID("content");
            creationDate = PMGD::StringID("creationDate");
            email = PMGD::StringID("email");
            emailaddress = PMGD::StringID("emailaddress");
            firstName = PMGD::StringID("firstName");
            gender = PMGD::StringID("gender");
            hasCreator = PMGD::StringID("hasCreator");
            hasInterest = PMGD::StringID("hasInterest");
            hasMember = PMGD::StringID("hasMember");
            hasTag = PMGD::StringID("hasTag");
            hasType = PMGD::StringID("hasType");
            id = PMGD::StringID("id");
            imageFile = PMGD::StringID("imageFile");
            isLocatedIn = PMGD::StringID("isLocatedIn");
            isPartOf = PMGD::StringID("isPartOf");
            isSubclassOf = PMGD::StringID("isSubclassOf");
            joinDate = PMGD::StringID("joinDate");
            knows = PMGD::StringID("knows");
            language = PMGD::StringID("language");
            lastName = PMGD::StringID("lastName");
            likes = PMGD::StringID("likes");
            locationIP = PMGD::StringID("locationIP");
            name = PMGD::StringID("name");
            replyOf = PMGD::StringID("replyOf");
            speaks = PMGD::StringID("speaks");
            studyAt = PMGD::StringID("studyAt");
            title = PMGD::StringID("title");
            type = PMGD::StringID("type");
            workAt = PMGD::StringID("workAt");
            workFrom = PMGD::StringID("workFrom");
        }
    }
};

extern strings_t strings;
