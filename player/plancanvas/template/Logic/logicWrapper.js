// 听牌(手牌, 立直状态, 场风, 役牌, 振听, 表宝牌)
const ting_pai = Logic.cwrap("ting_pai", "string", ["string", "number", "number", "string", "boolean", "string"]);