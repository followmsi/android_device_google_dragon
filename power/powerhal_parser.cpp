/*
 * Copyright (c) 2016-2017, NVIDIA CORPORATION.  All rights reserved.
 * Copyright (C) 2019 The LineageOS Project
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "powerHAL::parser"

#include <cstring>
#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <array>

#include <expat.h>

#include "powerhal_parser.h"
#include "powerhal_utils.h"
#include "powerhal.h"

using ::vendor::nvidia::hardware::power::V1_0::ExtPowerHint;

#define XML_FILE_PREFIX "powerhal."
#define XML_FILE_SUFFIX ".xml"
#define MAX_LINE_SIZE 256

static std::array<std::string, 4>  defaultXmlPath = {
{
    std::string("/odm/etc/"),
    std::string("/vendor/odm/etc/"),
    std::string("/vendor/etc/"),
    std::string("/system/etc/")
}
};

static int parse_int(const char *str, int *num)
{
    char *endstr;
    long val;

    val = strtol(str, &endstr, 10);
    if (str == endstr)
        return -1;
    if (val < -1 || val > INT_MAX)
        return -1;

    *num = (int)val;
    return 0;
}

static int get_bool(const char *type, const char *value)
{
    int val = -1;
    if (parse_int(value, &val) || val < 0) {
        ALOGE("%s is not a valid number", value);
        return -1;
    }

    if (!strcmp(type, "enable"))
        return !!val;

    ALOGE("Unknown attribute: %s", type);
    return -1;
}

static void set_hint_value(power_hint_data_t *hint, const char *type,
                        const char *value)
{
    int val = -1;
    if (parse_int(value, &val) || val < 0) {
        ALOGE("%s is not a valid number", value);
        return;
    }
    if (!strcmp(type, "min")) {
        hint->min = val;
    } else if (!strcmp(type, "max")) {
        hint->max = val;
    } else if (!strcmp(type, "duration")) {
        hint->time_ms = val;
    } else {
        ALOGE("Unknown attribute: %s", type);
    }
}

static void reset_hint(power_hint_data_t *hint)
{
    hint->min = 0;
    hint->max = INT_MAX;
    hint->time_ms = 0;
}

namespace {

const std::map<std::string, ExtPowerHint> power_hint_ids = {
        {"VSYNC",             ExtPowerHint::VSYNC},
        {"INTERACTION",       ExtPowerHint::INTERACTION},
        {"VIDEO_ENCODE",      ExtPowerHint::VIDEO_ENCODE},
        {"VIDEO_DECODE",      ExtPowerHint::VIDEO_DECODE},
        {"LOW_POWER",         ExtPowerHint::LOW_POWER},
        {"APP_PROFILE",       ExtPowerHint::APP_PROFILE},
        {"APP_LAUNCH",        ExtPowerHint::APP_LAUNCH},
        {"SHIELD_STREAMING",  ExtPowerHint::SHIELD_STREAMING},
        {"HIGH_RES_VIDEO",    ExtPowerHint::HIGH_RES_VIDEO},
        {"MIRACAST",          ExtPowerHint::MIRACAST},
        {"DISPLAY_ROTATION",  ExtPowerHint::DISPLAY_ROTATION},
        {"CAMERA",            ExtPowerHint::CAMERA},
        {"MULTITHREAD_BOOST", ExtPowerHint::MULTITHREAD_BOOST},
        {"AUDIO_SPEAKER",     ExtPowerHint::AUDIO_SPEAKER},
        {"AUDIO_OTHER",       ExtPowerHint::AUDIO_OTHER},
        {"POWER_MODE",        ExtPowerHint::POWER_MODE},
        {"AUDIO_LOW_LATENCY", ExtPowerHint::AUDIO_LOW_LATENCY}
};

class XmlElement {
    public:
        virtual ~XmlElement() {};
        virtual void parse(__attribute__((unused)) struct powerhal_info *pInfo, __attribute__((unused)) const char **attrs) {}
        virtual void finish(__attribute__((unused)) struct powerhal_info *pInfo) {}

        const std::string& name() const { return m_name; }
        XmlElement *parent() const { return m_parent; }

        XmlElement *find_child(std::string name) const {
            auto it = m_children.find(name);
            if (it == m_children.end()) {
                ALOGW("%s: unknown child element %s", m_name.c_str(), name.c_str());
                return NULL;
            }
            return it->second;
        }

    protected:
        XmlElement(XmlElement *parent,
                        std::map<std::string, XmlElement*> children,
                        std::string name) :
                m_parent(parent), m_children(children), m_name(name) {}

        XmlElement *const m_parent;
        const std::map<std::string, XmlElement*> m_children;
        const std::string m_name;
};

class XmlElementTop : public XmlElement {
    public:
        XmlElementTop(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "powerhal") {}
};

class XmlElementInputDevices : public XmlElement {
    public:
        XmlElementInputDevices(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "input_devices") {}
};

class XmlElementInput : public XmlElement {
    public:
        XmlElementInput(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "input") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            while (*attrs) {
                if (strcmp(attrs[0], "name")) {
                    ALOGE("Unknown input attribute: %s", attrs[0]);
                    attrs += 2;
                    continue;
                }
                const char *s = strdup(attrs[1]);
                if (!s) {
                    ALOGE("Couldn't copy %s: ", s);
                } else {
                    pInfo->input_devs.push_back({-1, s});
                }
                attrs += 2;
            }
        }
};

class XmlElementCpuCluster : public XmlElement {
    public:
        XmlElementCpuCluster(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "cpu_cluster") {}

        virtual void parse(struct powerhal_info *pInfo, __attribute__((unused)) const char **attrs) {
            pInfo->cpu_clusters.push_back({});
        }
};

class XmlElementCpuPmqosConstraint : public XmlElement {
    public:
        XmlElementCpuPmqosConstraint(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "pmqos_constraint") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            while (*attrs) {
                if (!strcmp(attrs[0], "path")) {
                    pInfo->cpu_clusters.back().pmqos_constraint_path =
                            strdup(attrs[1]);
                    if (!pInfo->cpu_clusters.back().pmqos_constraint_path)
                        ALOGE("Couldn't assign %s", attrs[1]);
                } else {
                    ALOGE("Unknown pmqos_constraint attribute: %s", attrs[0]);
                }
                attrs += 2;
            }
        }
};

class XmlElementCpuAvailableFreqs : public XmlElement {
    public:
        XmlElementCpuAvailableFreqs(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "available_freqs") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            while (*attrs) {
                if (!strcmp(attrs[0], "path")) {
                    pInfo->cpu_clusters.back().available_freqs_path =
                            strdup(attrs[1]);
                    if (!pInfo->cpu_clusters.back().available_freqs_path)
                        ALOGE("Couldn't assign %s", attrs[1]);
                } else {
                    ALOGE("Unknown available_freqs attribute: %s", attrs[0]);
                }
                attrs += 2;
            }
        }
};

class XmlElementHints : public XmlElement {
    public:
        XmlElementHints(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "hints") {}
};

class XmlElementHint : public XmlElement {
    public:
        XmlElementHint(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "hint") {}

        virtual void parse(__attribute__((unused)) struct powerhal_info *pInfo, const char **attrs) {
            m_hintId = static_cast<ExtPowerHint>(-1);
            for (; *attrs; attrs += 2) {
                if (strcmp(attrs[0], "name")) {
                    ALOGE("Unknown hint attribute: %s", attrs[0]);
                    continue;
                }
                auto it = power_hint_ids.find(attrs[1]);
                if (it == power_hint_ids.end()) {
                    ALOGW("couldn't find hint %s", attrs[1]);
                    continue;
                }
                m_hintId = it->second;
            }
        }

        virtual void finish(__attribute__((unused)) struct powerhal_info *pInfo) {
            m_hintId = static_cast<ExtPowerHint>(-1);
        }

        ExtPowerHint hintId() const {
            return m_hintId;
        }

    private:
        ExtPowerHint m_hintId;
};

class XmlElementHintInterval : public XmlElement {
    public:
        XmlElementHintInterval(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "interval") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            auto parent_hint = static_cast<XmlElementHint*>(m_parent);
            ExtPowerHint hint_id = parent_hint->hintId();
            if (hint_id < static_cast<ExtPowerHint>(0) || hint_id >= POWER_HINT_MAX) {
                ALOGE("Invalid hint id: %d", hint_id);
                return;
            }
            for (; *attrs; attrs += 2) {
                if (strcmp(attrs[0], "time")) {
                    ALOGE("Unknown interval attribute: %s", attrs[0]);
                    continue;
                }
                int interval = -1;
                if (parse_int(attrs[1], &interval) || interval < 0) {
                    ALOGE("%s is not a valid interval", attrs[1]);
                    continue;
                }
                pInfo->hint_interval[hint_id] = interval;
            }
        }
};

class XmlElementHintCpu : public XmlElement {
    public:
        XmlElementHintCpu(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "cpu") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            auto parent_hint = static_cast<XmlElementHint*>(m_parent);
            ExtPowerHint hint_id = parent_hint->hintId();
            if (hint_id < static_cast<ExtPowerHint>(0) || hint_id >= POWER_HINT_MAX) {
                ALOGE("Invalid hint id: %d", hint_id);
                return;
            }
            int cluster = -1;
            if (!strcmp(attrs[0], "cluster")) {
                if (parse_int(attrs[1], &cluster) || cluster < 0) {
                    ALOGE("%s is not a valid number", attrs[1]);
                }
                attrs += 2;
            }
            if (cluster >= 0 && (size_t)cluster >= pInfo->cpu_clusters.size()) {
                ALOGE("Invalid cluster id: %d", cluster);
                return;
            }
            if (cluster < 0) {
                for (auto &cpu_cluster : pInfo->cpu_clusters) {
                    reset_hint(&cpu_cluster.hints[hint_id]);
                }
            } else {
                reset_hint(&pInfo->cpu_clusters[cluster].hints[hint_id]);
            }
            for (; *attrs; attrs += 2) {
                if (!strcmp(attrs[0], "cluster")) {
                    ALOGE("cluster attribute should come before others");
                    continue;
                }
                if (cluster < 0) {
                    for (auto &cpu_cluster : pInfo->cpu_clusters) {
                        set_hint_value(&cpu_cluster.hints[hint_id],
                                        attrs[0], attrs[1]);
                    }
                } else {
                    set_hint_value(&pInfo->cpu_clusters[cluster].hints[hint_id],
                                    attrs[0], attrs[1]);
                }
            }
        }
};

class XmlElementHintGpu : public XmlElement {
    public:
        XmlElementHintGpu(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "gpu") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            auto parent_hint = static_cast<XmlElementHint*>(m_parent);
            ExtPowerHint hint_id = parent_hint->hintId();
            if (hint_id < static_cast<ExtPowerHint>(0) || hint_id >= POWER_HINT_MAX) {
                ALOGE("Invalid hint id: %d", hint_id);
                return;
            }
            reset_hint(&pInfo->gpu_freq_hints[hint_id]);
            for (; *attrs; attrs += 2) {
                set_hint_value(&pInfo->gpu_freq_hints[hint_id], attrs[0], attrs[1]);
            }
        }
};

class XmlElementHintEmc : public XmlElement {
    public:
        XmlElementHintEmc(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "emc") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            auto parent_hint = static_cast<XmlElementHint*>(m_parent);
            ExtPowerHint hint_id = parent_hint->hintId();
            if (hint_id < static_cast<ExtPowerHint>(0) || hint_id >= POWER_HINT_MAX) {
                ALOGE("Invalid hint id: %d", hint_id);
                return;
            }
            reset_hint(&pInfo->emc_freq_hints[hint_id]);
            for (; *attrs; attrs += 2) {
                set_hint_value(&pInfo->emc_freq_hints[hint_id], attrs[0], attrs[1]);
            }
        }
};

class XmlElementHintOnlineCpus : public XmlElement {
    public:
        XmlElementHintOnlineCpus(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "online_cpus") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            auto parent_hint = static_cast<XmlElementHint*>(m_parent);
            ExtPowerHint hint_id = parent_hint->hintId();
            if (hint_id < static_cast<ExtPowerHint>(0) || hint_id >= POWER_HINT_MAX) {
                ALOGE("Invalid hint id: %d", hint_id);
                return;
            }
            reset_hint(&pInfo->online_cpu_hints[hint_id]);
            for (; *attrs; attrs += 2) {
                set_hint_value(&pInfo->online_cpu_hints[hint_id], attrs[0], attrs[1]);
            }
        }
};

class XmlElementBootBoost : public XmlElement {
    public:
        XmlElementBootBoost(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "boot_boost") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            for (; *attrs; attrs += 2) {
                if (strcmp(attrs[0], "time")) {
                    ALOGE("Unknown boot_boost attribute: %s", attrs[0]);
                    continue;
                }
                int time = -1;
                if (parse_int(attrs[1], &time) || time < 0) {
                    ALOGE("%s is not a valid time", attrs[1]);
                    continue;
                }
                pInfo->boot_boost_time_ms = time;
            }
        }
};

class XmlElementCpufreqInteractive : public XmlElement {
    public:
        XmlElementCpufreqInteractive(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "cpufreq_interactive") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            for (; *attrs; attrs += 2) {
                int b = get_bool(attrs[0], attrs[1]);
                if (b < 0)
                    continue;
                pInfo->no_cpufreq_interactive = !b;
            }
        }
};

class XmlElementSclkBoost : public XmlElement {
    public:
        XmlElementSclkBoost(XmlElement *parent,
                        std::map<std::string, XmlElement*> children) :
                XmlElement(parent, children, "sclk_boost") {}

        virtual void parse(struct powerhal_info *pInfo, const char **attrs) {
            for (; *attrs; attrs += 2) {
                int b = get_bool(attrs[0], attrs[1]);
                if (b < 0)
                    continue;
                pInfo->no_sclk_boost = !b;
            }
        }
};
// These externs are necessary so that we can have circular parent/children
// pointers.
extern XmlElementBootBoost xml_boot_boost;
extern XmlElementCpuAvailableFreqs xml_cpu_available_freqs;
extern XmlElementCpuCluster xml_cpu_cluster;
extern XmlElementCpuPmqosConstraint xml_cpu_pmqos_constraint;
extern XmlElementCpufreqInteractive xml_cpufreq_interactive;
extern XmlElementHint xml_hint;
extern XmlElementHintCpu xml_hint_cpu;
extern XmlElementHintEmc xml_hint_emc;
extern XmlElementHintGpu xml_hint_gpu;
extern XmlElementHintInterval xml_hint_inverval;
extern XmlElementHintOnlineCpus xml_hint_online_cpus;
extern XmlElementHints xml_hints;
extern XmlElementInput xml_input;
extern XmlElementInputDevices xml_input_devices;
extern XmlElementSclkBoost xml_sclk_boost;
extern XmlElementTop xml_top;

// These must be defined in bottom-to-top order, so that .name() will work
// correctly.
XmlElementHintInterval xml_hint_inverval(&xml_hint, {});
XmlElementHintCpu xml_hint_cpu(&xml_hint, {});
XmlElementHintGpu xml_hint_gpu(&xml_hint, {});
XmlElementHintEmc xml_hint_emc(&xml_hint, {});
XmlElementHintOnlineCpus xml_hint_online_cpus(&xml_hint, {});
XmlElementHint xml_hint(&xml_hints, {
                {xml_hint_inverval.name(), &xml_hint_inverval},
                {xml_hint_cpu.name(), &xml_hint_cpu},
                {xml_hint_gpu.name(), &xml_hint_gpu},
                {xml_hint_emc.name(), &xml_hint_emc},
                {xml_hint_online_cpus.name(), &xml_hint_online_cpus}
                });
XmlElementHints xml_hints(&xml_top, {
                {xml_hint.name(), &xml_hint}
                });
XmlElementCpuAvailableFreqs xml_cpu_available_freqs(&xml_cpu_cluster, {});
XmlElementCpuPmqosConstraint xml_cpu_pmqos_constraint(&xml_cpu_cluster, {});
XmlElementCpuCluster xml_cpu_cluster(&xml_top, {
                {xml_cpu_available_freqs.name(), &xml_cpu_available_freqs},
                {xml_cpu_pmqos_constraint.name(), &xml_cpu_pmqos_constraint}
                });
XmlElementInput xml_input(&xml_input_devices, {});
XmlElementInputDevices xml_input_devices(&xml_top, {
                {xml_input.name(), &xml_input}
                });
XmlElementBootBoost xml_boot_boost(&xml_top, {});
XmlElementCpufreqInteractive xml_cpufreq_interactive(&xml_top, {});
XmlElementSclkBoost xml_sclk_boost(&xml_top, {});
XmlElementTop xml_top(NULL, {
                {xml_boot_boost.name(), &xml_boot_boost},
                {xml_cpu_cluster.name(), &xml_cpu_cluster},
                {xml_cpufreq_interactive.name(), &xml_cpufreq_interactive},
                {xml_hints.name(), &xml_hints},
                {xml_input_devices.name(), &xml_input_devices},
                {xml_sclk_boost.name(), &xml_sclk_boost}
                });
}

struct Data {
    XML_Parser parser;
    XmlElement *element;
    std::stack<std::string> elements;
    struct powerhal_info *pInfo;

    bool abort;
};

static void elementStart(void* data_, const char* _name, const char **attrs)
{
    auto data = static_cast<Data*>(data_);
    std::string name = std::string(_name);
    XmlElement *child = NULL;

    if (data->elements.empty()) {
        if (name != "powerhal") {
            ALOGE("Root element should be <powerhal>!");
            data->abort = true;
            XML_SetElementHandler(data->parser, NULL, NULL);
            return;
        }
        data->elements.push(std::move(name));
        return;
    }

    std::string &parent = data->elements.top();

    if (parent == data->element->name()) {
        child = data->element->find_child(name);
        if (child) {
            child->parse(data->pInfo, attrs);
        } else {
            ALOGE("Unknown element: %s", name.c_str());
        }
    }

    if (child)
        data->element = child;
    data->elements.push(std::move(name));
}

static void elementEnd(void* data_, __attribute__((unused)) const char* name)
{
    auto data = static_cast<Data*>(data_);

    if (data->elements.top() == data->element->name()) {
        data->element->finish(data->pInfo);
        data->element = data->element->parent();
    }
    data->elements.pop();
}

int parse_xml(struct powerhal_info *pInfo, const char *hw_name)
{
    char buf[MAX_LINE_SIZE];
    int ret = 0;
    struct Data data;
    XML_Parser parser;

    std::string filename;
    int i;

    for (i = 0; i < static_cast<int>(defaultXmlPath.size()); i++) {
        filename = defaultXmlPath[i] + std::string(XML_FILE_PREFIX)
                + std::string(hw_name) + std::string(XML_FILE_SUFFIX);
        if (0 == access(filename.c_str(), R_OK))
            break;
    }

    if (i >= static_cast<int>(defaultXmlPath.size())) {
        ALOGE("Couldn't find default xml file!");
        return -1;
    }

    ALOGI("Reading xml file %s", filename.c_str());
    std::ifstream file(filename, std::ifstream::in);

    if (!file.is_open()) {
        ALOGE("Couldn't open xml file %s", filename.c_str());
        return -1;
    }

    parser = XML_ParserCreate(nullptr);
    if (!parser) {
        ALOGE("Couldn't create XML parser");
        return -1;
    }

    data.pInfo = pInfo;
    data.abort = false;
    data.parser = parser;
    data.element = &xml_top;
    XML_SetUserData(parser, &data);
    XML_SetElementHandler(parser, elementStart, elementEnd);
    do {
        file.read(buf, sizeof(buf));
        if (XML_Parse(parser, buf, file.gcount(), !file) != XML_STATUS_OK) {
            ALOGE("Error parsing XML: %s", XML_ErrorString(XML_GetErrorCode(parser)));
            ret = -1;
            break;
        }
        if (data.abort)
            break;
    } while (file);

    XML_ParserFree(parser);

    return ret;
}
